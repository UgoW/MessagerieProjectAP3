cmake_minimum_required(VERSION 3.26)
project(MessagerieSec C)

set(CMAKE_C_STANDARD 11)

add_executable(MessagerieSec main.c
        Crypto/rsaEncryption.c
        Crypto/rsaEncryption.h
        Crypto/aesEncryption.cpp)
