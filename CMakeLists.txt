cmake_minimum_required(VERSION 3.14)
project(video-automation)
set(CMAKE_C_STANDARD 11)

add_subdirectory(lib/FFmpeg)

add_definitions(-DGL_SILENCE_DEPRECATION)

add_executable(video-app /remux.c)
target_link_libraries(video-app FFMPEG)