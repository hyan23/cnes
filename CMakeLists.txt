cmake_minimum_required(VERSION 2.8)

project(cnes)

set(CMAKE_C_FLAGS "-std=c99 -Wall")

add_subdirectory(src/2A03)
add_subdirectory(src/2C02)
add_subdirectory(src/6502)
add_subdirectory(src/common)
add_subdirectory(src/ini)
add_subdirectory(src/input)
add_subdirectory(src/mmc)
add_subdirectory(src/nes)
add_subdirectory(src/rom)

add_executable(cnes src/cnes.c)

target_include_directories(cnes PRIVATE include src/common)
target_link_libraries(cnes PRIVATE nes)
target_link_libraries(cnes PRIVATE SDL m)

# TODO: install
