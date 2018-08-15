#!/usr/bin/env bash


compile="Release"



export GCC_HOME=/usr/bin

#build target project
cmake -G"Unix Makefiles" -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_BUILD_TYPE=$compile \
    -DCMAKE_CXX_COMPILER=$GCC_HOME/arm-linux-gnueabihf-g++-4.9 \
    -DCMAKE_C_COMPILER=$GCC_HOME/arm-linux-gnueabihf-gcc-4.9 \
    -DCMAKE_AR=$GCC_HOME/arm-linux-gnueabihf-ar -DPLATFORM=arm \
    -DCMAKE_SYSTEM_PROCESSOR=arm-linux \
    -DCMAKE_INSTALL_PREFIX=/home/sftantp/Downloads/libjpeg-turbo/build/install \
    -DWITH_JPEG8=1 \
    .. 




