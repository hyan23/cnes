file(GLOB SOURCES "*.c")

add_library(rom STATIC ${SOURCES})

target_include_directories(rom PRIVATE ../../include ../common)
target_link_libraries(rom PRIVATE common)
