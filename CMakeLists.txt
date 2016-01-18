cmake_minimum_required(VERSION 3.3)
project(IRCChat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Werror")

set(SOURCE_FILES main.c protocol.h serveur-udp.c)
add_executable(IRCChat ${SOURCE_FILES})