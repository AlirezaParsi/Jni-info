cmake_minimum_required(VERSION 3.10.2)

project("copg")

add_library(
    copg
    SHARED
    src/main/jni/copg_jni.c
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(
    copg
    ${log-lib}
    ${android-lib}
)
