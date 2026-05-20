cmake_minimum_required(VERSION 3.22)
project(wydra_apk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Android NDK configuration
set(ANDROID_ABI arm64-v8a)
set(ANDROID_PLATFORM android-35)
set(ANDROID_STL c++_shared)

# Source files
set(SOURCES
    src/main/jni/native-lib.c
)

# Create native library
add_library(wydra_native SHARED ${SOURCES})

# Include directories
target_include_directories(wydra_native PRIVATE
    src/main/jni/include
)

# Link Android libraries
target_link_libraries(wydra_native
    android
    log
)
