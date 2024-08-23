include_directories(${CMAKE_SOURCE_DIR}/external)

file(GLOB_RECURSE INIH_SOURCE_LIST
    ${CMAKE_SOURCE_DIR}/external/inih/ini.c
    ${CMAKE_SOURCE_DIR}/external/inih/ini.h
    ${CMAKE_SOURCE_DIR}/external/inih/cpp/INIReader.h
    ${CMAKE_SOURCE_DIR}/external/inih/cpp/INIReader.cpp
)

add_library(INIH_LIB STATIC ${INIH_SOURCE_LIST})
