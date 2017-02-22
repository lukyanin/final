cmake_minimum_required(VERSION 2.8)
project(final)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp WebServer.cpp WebServer.h HttpRequest.cpp HttpRequest.h Utils.cpp Utils.h HttpResponse.cpp HttpResponse.h)
add_executable(final ${SOURCE_FILES})
