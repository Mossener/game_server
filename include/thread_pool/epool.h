#ifndef EPOOL_H
#define EPOOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <atomic>
#include <future>
#include <functional>
#include <stdexcept>

class Epool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> pool;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop{false};

public:
    explicit Epool(size_t size = std::thread::hardware_concurrency()) {
        if (size == 0) size = 1;
        for (size_t i = 0; i < size; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->mtx);
                        this->cv.wait(lock, [this] {
                            return this->stop.load() || !this->pool.empty();
                        });

                        if (this->stop.load() && this->pool.empty())
                            return;

                        task = std::move(this->pool.front());
                        this->pool.pop();
                    }
                    try {
                        task();
                    } catch (const std::exception &e) {
                        // 可选：日志或忽略
                    }
                }
            });
        }
    }

    ~Epool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop.store(true);
        }
        cv.notify_all();
        for (auto &worker : workers)
            if (worker.joinable())
                worker.join();
    }

    template <typename F, typename... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {

        using return_type = typename std::invoke_result<F, Args...>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (stop.load())
                throw std::runtime_error("enqueue on stopped thread pool");

            pool.emplace([task]() { (*task)(); });
        }
        cv.notify_one();
        return res;
    }
};

#endif // EPOOL_H
