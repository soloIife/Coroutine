cmake_minimum_required(VERSION 3.16)
project(MyThread)

set(CMAKE_CXX_STANDARD 14)

add_executable(test00 main.cpp coroutine.cpp stdafx.h coroutine.h)