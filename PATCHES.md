Planned code patch list (for review)
===================================

Summary
-------
下面是我打算对项目应用的一组小而安全的补丁，目标是先解决最关键的运行时问题（SIGPIPE 导致进程退出、阻塞 socket、无法优雅关闭、以及在 UI 输出中使用 system("clear") 的问题）。每个补丁包含要修改（或新增）的文件、简要理由、以及建议的代码变更片段。你确认后我会按你同意的补丁逐一应用（并在每步等待你确认）。

总体目标（最小安全修复，推荐先应用）
- 忽略 SIGPIPE 防止 send 导致进程被信号中止
- 把监听 socket 与 accepted client sockets 设置为非阻塞（O_NONBLOCK）
- 添加优雅关闭入口：signal handler + Server::stop() 或等价的 running 标志，使 epoll loop 可退出并清理
- 去掉 system("clear")（或仅在开发模式下打印而不要调用 system）

Patch 1 — 忽略 SIGPIPE（文件：app/main.cpp）
- 原因：当对端在写入前关闭连接时，send 可能触发 SIGPIPE 并终止进程。通过忽略 SIGPIPE 可以让 send 返回错误 (-1) 并由程序处理。
- 修改说明（短）：
  - 在头部添加 #include <signal.h>
  - 在 main 函数启动前或开始处调用 signal(SIGPIPE, SIG_IGN);
- 代码示例（将替换 app/main.cpp 顶部/main 初始化相关部分）：
  - Add:
    #include <signal.h>
  - In main():
    signal(SIGPIPE, SIG_IGN);

Patch 2 — 监听 socket 与 accepted sockets 设为非阻塞（文件：include/my_net/my_socket.h, src/server.cpp）
- 原因：阻塞 socket 在基于 epoll 的服务器中可能导致阻塞行为，使用非阻塞更易与 epoll（尤其是 EPOLLET）配合。
- 修改说明：
  - 在 MySocket 构造中或在 bind/listen 完成后为 listen socket 设置 O_NONBLOCK。
  - 在 acceptConnection() 返回 clientfd 之前，设置 clientfd 为 O_NONBLOCK。
- 代码片段（在 include/my_net/my_socket.h 的相关方法中）：
  - Add helper:
    #include <fcntl.h> // already included
    static void setNonBlockingFD(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) flags = 0;
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
  - In constructor (after socket created):
    setNonBlockingFD(sockfd);
  - In acceptConnection() after accept returns clientfd:
    setNonBlockingFD(clientfd);

Patch 3 — 添加优雅关闭支持（文件：include/init/server.h, src/server.cpp, app/main.cpp）
- 原因：当前主循环为 while(true) 且会无限运行，不处理 SIGINT/SIGTERM，导致无法优雅释放 epoll、关闭 fds、终止线程池、flush 日志等。
- 修改说明（高层）：
  - 在 Server 类中增加成员 std::atomic<bool> running_ (或使用全局 atomic) 与方法 void stop();
  - 在 start() 的主循环里使用 while (running_) 而不是 while (true)。
  - Server::stop() 将设置 running_=false，并关闭 epoll_fd（或使用 eventfd/wakeup 机制将 epoll_wait 唤醒）；并可关闭所有 client fds（或留由析构处理）。
  - 在 app/main.cpp 注册 SIGINT / SIGTERM handler，调用 server.stop()（或设置全局 flag）。
- 代码示例（关键片段）：
  - include/init/server.h:
    + add: #include <atomic>
    + add to class: std::atomic<bool> running_{true};
    + add method: void stop();
  - src/server.cpp:
    + change loop: while (running_) { ... }
    + implement stop():
        void Server::stop() {
            running_.store(false);
            // optionally: wake up epoll by closing epoll_fd or write to eventfd if used
        }
  - app/main.cpp:
    + register handler that calls server.stop() (needs server instance to be accessible from handler; can use static/global pointer or lambda capturing - simpler: use a global pointer to server and set it in main).

Notes: there are multiple approaches to "唤醒 epoll_wait": closing epoll_fd from another thread makes epoll_wait return with error; using eventfd or pipe is cleaner. For minimal change we can call close(epoll_fd) in stop(); epoll_wait will return -1 and start() can check running_ and exit gracefully. Ensure epoll_fd is accessible in stop().

Patch 4 — 移除 system("clear")（文件：src/server.cpp）
- 原因：system 调用代价大且不适合后台服务；它在输出清单用处有限且会影响可移植性。
- 修改说明：
  - 在 printfd() 中移除 system("clear"); 或替换为简单的打印分隔符。
- 代码示例：
  - Replace:
    system("clear");
    std::cout << "Current connected client fds:" << std::endl;
  - With:
    std::cout << "---- Current connected client fds ----" << std::endl;

Patch 5 (optional) — 在 read/send 处更稳健地处理 EINTR/EAGAIN（文件：include/my_net/http.h, src/server.cpp）
- 原因：网络 IO 需要处理 EINTR/EAGAIN，尤其在非阻塞模式下应检测 EAGAIN 并返回重试逻辑。
- 修改说明（建议但可后置）：
  - 在发送 send loop 中已处理 EINTR。对 read/recv 使用循环，处理 EINTR；对非阻塞 fd 的 EAGAIN 应当停止读取并等待下一次 epoll 通知。
- 建议实施步骤：把 HttpHandle::HttpHandle_f 中的 read/send 保持但在遇到 EAGAIN 时不抛异常，而是返回或适应非阻塞逻辑（可能需要更复杂的 state machine）。

Patch 6 (optional, medium) — 统一日志（移除自定义 Logger 或将其作为示例）
- 原因：项目同时用了 spdlog 的异步 logger 又有自写 Logger，可能造成混淆与重复。我建议先保留 spdlog，并把自写 Logger 作为示例代码（不必在 Server 中混用）。
- 修改说明：不在本轮必做，若你同意可在后续合并/删除自实现 Logger 文件。

具体变更清单（按文件）
- app/main.cpp
  - 添加 #include <signal.h>
  - 在 main 开头调用 signal(SIGPIPE, SIG_IGN);
  - 注册 SIGINT/SIGTERM handler（或使用全局 pointer + handler 调用 server.stop()）
- include/init/server.h
  - 添加 <atomic>
  - 增加 running_ 成员与 void stop() 方法声明
- src/server.cpp
  - 将主循环由 while(true) 改为 while(running_)
  - 实现 Server::stop()（设置 running_ = false；并做 epoll 唤醒/close）
  - 在 printfd() 中去掉 system("clear")
- include/my_net/my_socket.h
  - 在构造后与 accept 后设置 O_NONBLOCK（使用 fcntl）
  - （保留现有 API，行为向后兼容）
- include/my_net/http.h (可选)
  - 在 read/send 的边界错误时更稳健地处理 EAGAIN/EINTR（建议逐步改）。

Applying strategy
---------------
- 我会按补丁编号逐步应用：先做 Patch 1、Patch 2、Patch 4、Patch 3（因为 stop() 需要非阻塞 socket 来更平滑地退出），并在每次修改后请求你确认构建/运行情况。
- 每次实际代码修改我将用精确的 replace_in_file 调整特定行（不会一次覆盖整个文件，除非需要）。修改后你会看到自动格式化的结果并确认。

下一步
-------
请选择：
- [ ] 同意按上面顺序应用补丁（我将从 Patch 1 开始并逐步提交每个修改）
- [ ] 只应用部分补丁（请列出想要的补丁编号，例如 "1,2,4"）
- [ ] 我需要你先把每个补丁的实际 diff（SEARCH/REPLACE 块形式）显示出来以便逐条审阅

我已把当前计划写在此文件中，确认你的选择后我会执行第一个代码修改（并在执行前再次列出将变更的实际 SEARCH/REPLACE 块）。
