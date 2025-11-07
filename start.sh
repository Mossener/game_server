#!/bin/bash
set -euo pipefail

# === 基本配置 ===
BUILD_TARGET="build"
TARGET="HttpServer"
LOGGER_DIR="$BUILD_TARGET/logger"
DEFAULT_BUILD_TYPE="release"

# === 日志输出函数 ===
info()    { echo -e "\033[1;34m[INFO]\033[0m $*"; }
warning() { echo -e "\033[1;33m[WARN]\033[0m $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*" >&2; }
success() { echo -e "\033[1;32m[SUCCESS]\033[0m $*"; }

# === 命令行参数解析 ===
BUILD_F="${1:-build}"
BUILD_TYPE="${2:-$DEFAULT_BUILD_TYPE}"
BUILD_TYPE=$(echo "$BUILD_TYPE" | tr '[:lower:]' '[:upper:]')

# === 功能函数 ===
configure() {
  info "执行 CMake 配置：$BUILD_TYPE 模式"
  cmake -S . -B "$BUILD_TARGET" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
}

build() {
  info "开始编译 CMake 项目"
  cmake --build "$BUILD_TARGET" -j"$(nproc)"
  success "成功构建"
}

clean() {
  if [[ -d "$BUILD_TARGET" ]]; then
    warning "删除旧的构建目录：$BUILD_TARGET"
    rm -rf "$BUILD_TARGET"
  else
    info "未检测到构建目录"
  fi
  success "清理完成"
}

run() {
  info "即将运行 $TARGET 程序"
  cd "$BUILD_TARGET"
  if [[ -x "./$TARGET" ]]; then 
    success "开始运行"
    "./$TARGET"
  else
    error "未找到可执行程序：$TARGET"
    exit 1
  fi
}

prepare_dirs() {
  mkdir -p "$LOGGER_DIR"
}

# === 主逻辑 ===
printf "\n"
case "$BUILD_F" in
  clean)
    clean
    ;;
  build)
    clean
    prepare_dirs
    configure
    build
    ;;
  run)
    run
    ;;
  rebuild)
    clean
    prepare_dirs
    configure
    build
    run
    ;;
  *)
    echo "用法: $0 [clean|run|build|rebuild] [debug|release]"
    exit 1
    ;;
esac
