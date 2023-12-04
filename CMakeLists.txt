cmake_minimum_required(VERSION 3.27)

project(signal_proc VERSION 1.0 LANGUAGES CXX)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(STB_DIR ${PROJECT_SOURCE_DIR}/third_party/stb)

set(SOURCE
    ${SRC_DIR}/main.cpp
)

add_executable(signal_proc ${SOURCE})

target_include_directories(signal_proc PRIVATE ${SRC_DIR} ${STB_DIR})

file(COPY ${PROJECT_SOURCE_DIR}/img/ DESTINATION ${PROJECT_BINARY_DIR}/)