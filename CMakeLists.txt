cmake_minimum_required(VERSION 3.15)
project(HttpServert)

# 自动检测并设置 vcpkg toolchain，仅在未定义时生效
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "/home/yurika/c_area/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logger)
add_definitions(-DLOGGER_PATH="${CMAKE_BINARY_DIR}/logger")
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(HttpServer       

 ${SRC_FILES} 
 ${CMAKE_SOURCE_DIR}/app/main.cpp) 
target_include_directories(HttpServer PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(HttpServer fmt::fmt spdlog::spdlog)
 