cmake_minimum_required(VERSION 3.19)
project(gm82cs CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "/O2 /W2 /WX")

add_library(gm82cs SHARED gm82cs.cpp)
