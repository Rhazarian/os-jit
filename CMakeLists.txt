cmake_minimum_required(VERSION 3.10)
project(brainfuckjit)

set(CMAKE_CXX_STANDARD 17)

add_executable(brainfuckjit brainfuck_program.h brainfuck_program.cpp main.cpp)