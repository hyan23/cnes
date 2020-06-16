set(SOURCES 6502.c cpu.c instructions.c memory.c)

add_library(6502 STATIC ${SOURCES})

target_compile_definitions(6502 PRIVATE O3)
target_include_directories(6502 PRIVATE ../../include ../common)
