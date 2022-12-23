cmake_minimum_required(VERSION 3.24)
project(real_time_log_system_package C)

set(CMAKE_C_STANDARD 11)

include_directories(rtlsp)

add_executable(real_time_log_system_package
        example_app/main.c
        rtlsp/message.h
        rtlsp/rtlsp.c
        rtlsp/rtlsp.h)
