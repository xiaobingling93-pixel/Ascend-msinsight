cmake_minimum_required(VERSION 3.16)
project(sqlite)

set(CMAKE_CXX_STANDARD 17)

include_directories(./include)
aux_source_directory(./src SRC)

add_library(${PROJECT_NAME} ${SRC})