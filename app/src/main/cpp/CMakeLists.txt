cmake_minimum_required(VERSION 3.22.1)
project("appdebugtoolkit")

# Enable C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add the complete toolkit source
add_library(
    appdebugtoolkit
    SHARED
    app-debug-toolkit.c
)

# Link required libraries
target_link_libraries(
    appdebugtoolkit
    log
    dl
    android
)

# ARM64 specific flags
target_compile_options(
    appdebugtoolkit
    PRIVATE
    -D_GNU_SOURCE
    -Wall
    -Wextra
    -O2
    -fvisibility=hidden
)

# Set ARM64 ABI
set_target_properties(
    appdebugtoolkit
    PROPERTIES
    ANDROID_ABI "arm64-v8a"
    ANDROID_STL "c++_shared"
)
