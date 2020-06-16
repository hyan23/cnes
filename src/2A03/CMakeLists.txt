file(GLOB SOURCES "*.c")

add_library(2A03 STATIC ${SOURCES})

target_compile_definitions(2A03 PRIVATE O3)
target_include_directories(2A03 PRIVATE ../../include ../common)
