# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 项目信息
project (Demo1)
# 指定生成目标
#INCLUDE_DIRECTORIES("./" 
#    "/opt/libjpeg-turbo/include/")
#LINK_DIRECTORIES("/opt/libjpeg-turbo/lib64/")

INCLUDE_DIRECTORIES("./" 
    "/home/sftantp/Downloads/libjpeg-turbo/build/install/include/")
LINK_DIRECTORIES("/home/sftantp/Downloads/libjpeg-turbo/build/install/lib/")
add_executable(Demo V4l2Camera.cpp test.cpp)
TARGET_LINK_LIBRARIES(Demo turbojpeg)

