cmake_minimum_required(VERSION 2.8)

project(mpp_linux_demo)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp
        mpp/Codec.cpp
        thread/Thread.cpp
        rkrga/RGA.cpp
        rkdrm/bo.c
        rkdrm/dev.c
        rkdrm/modeset.c)

add_executable(mpp_linux_demo ${SOURCE_FILES})
target_link_libraries(mpp_linux_demo mpp pthread drm)