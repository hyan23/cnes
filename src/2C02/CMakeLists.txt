file(GLOB SOURCES "*.c")

add_library(2C02 STATIC ${SOURCES})

target_compile_definitions(2C02 PRIVATE O3)
target_include_directories(2C02 PRIVATE ../../include ../common)
