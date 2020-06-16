file(GLOB SOURCES "*.c")

add_library(input STATIC ${SOURCES})

target_include_directories(input PRIVATE ../../include ../common)
