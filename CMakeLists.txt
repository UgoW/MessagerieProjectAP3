cmake_minimum_required(VERSION 3.26)
project(MessagerieSec C)

set(CMAKE_C_STANDARD 11)

add_executable(MessagerieSec
        Crypto/rsaEncryption.c
        Crypto/aesEncryption.cpp
        server.c
        Client.c)
