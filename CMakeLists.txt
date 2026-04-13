cmake_minimum_required(VERSION 3.15)

project(Pong)

find_package(raylib REQUIRED)
add_library(pongLib STATIC src/pong.c)

target_include_directories(pongLib PUBLIC include)

add_executable(pong src/main.c)

target_link_libraries(pong raylib)
target_link_libraries(pong pongLib)
