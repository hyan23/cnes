file(GLOB SOURCES "*.c")

add_library(nes STATIC ${SOURCES})

target_include_directories(nes PRIVATE ../../include ../common)
target_link_libraries(nes PRIVATE 2A03 2C02 6502 common ini input mmc rom)
