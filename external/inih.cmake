file(GLOB_RECURSE inih_SOURCE_LIST
    ${CMAKE_SOURCE_DIR}/external/inih/ini.c
    ${CMAKE_SOURCE_DIR}/external/inih/ini.h
    ${CMAKE_SOURCE_DIR}/external/inih/cpp/INIReader.h
    ${CMAKE_SOURCE_DIR}/external/inih/cpp/INIReader.cpp
)

add_library(inih-static STATIC ${inih_SOURCE_LIST})
target_compile_definitions(inih-static PRIVATE INI_ALLOW_MULTILINE=0)
add_library(inih::static ALIAS inih-static)
