cmake_minimum_required(VERSION 3.17)
project(SimConnectTest LANGUAGES CXX VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories(
        "$ENV{MSFS_SDK}/SimConnect SDK/include"
        ${CMAKE_SOURCE_DIR}/src
)

link_directories(
        "$ENV{MSFS_SDK}/SimConnect SDK/lib"
)

add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -DNOGDI
)

add_subdirectory(src)
