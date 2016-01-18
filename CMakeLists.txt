cmake_minimum_required(VERSION 2.8)
project(IRCChat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Werror")

set(SOURCE_FILES client.c protocol.h)
add_executable(IRCChat ${SOURCE_FILES})
