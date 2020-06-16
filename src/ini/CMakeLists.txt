set(SOURCES ini.c pair.c section.c)

add_library(ini STATIC ${SOURCES})

target_include_directories(ini PRIVATE ../../include ../common)
target_link_libraries(ini PRIVATE common)
