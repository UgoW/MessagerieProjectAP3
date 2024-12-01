cmake_minimum_required(VERSION 3.26)
project(MessagerieProjectAP3 C)

set(CMAKE_C_STANDARD 11)


# Add exexutable client
add_executable(client client.c)
# Add executable server
add_executable(server server.c)

