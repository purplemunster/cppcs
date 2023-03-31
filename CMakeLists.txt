cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)

project(cppcs)

add_subdirectory(imported/glm)

add_executable(cppcs main.cpp byte_address_buffer.h dispatch.h)
target_link_libraries(cppcs glm)