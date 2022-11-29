cmake_minimum_required(VERSION 3.21)
project(compiler)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_subdirectory(frontend)
add_subdirectory(middle)
add_subdirectory(backend)

add_executable(compiler main.cpp)
target_link_libraries(compiler FRONT)
target_link_libraries(compiler MIDDLE)
target_link_libraries(compiler BACK)
