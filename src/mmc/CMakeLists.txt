file(GLOB SOURCES "*.c")

add_library(mmc STATIC ${SOURCES})

target_include_directories(mmc PRIVATE ../../include ../common)
