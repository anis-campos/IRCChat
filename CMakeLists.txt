cmake_minimum_required(VERSION 2.8)
project(IRCChat)

find_package (Threads)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Werror")

set(SOURCE_FILES client.c protocol.h)

add_executable(ServeurIRCChat serveur-udp.c protocol.h)
add_executable(ClientIRCChat ${SOURCE_FILES})


target_link_libraries (ClientIRCChat ${CMAKE_THREAD_LIBS_INIT})

