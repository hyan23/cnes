file(GLOB SOURCES "*.c")

add_library(common STATIC ${SOURCES})

target_compile_definitions(common PRIVATE O3)
target_include_directories(common PRIVATE ../../include ../common)
