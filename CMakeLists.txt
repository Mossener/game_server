cmake_minimum_required(VERSION 3.15)
PROJECT(ExampleProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logger)
add_definitions(-DLOGGER_PATH="${CMAKE_BINARY_DIR}/logger")
include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(logger_app       

 ${SRC_FILES} 
 ${CMAKE_SOURCE_DIR}/app/main.cpp)
