cmake_minimum_required(VERSION 3.10.2)

project("copg")

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    copg
    SHARED
    src/main/jni/copg_jni.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(
    copg
    ${log-lib}
    ${android-lib}
)
