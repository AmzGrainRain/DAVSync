file(GLOB_RECURSE inih_SOURCE_LIST
    ${CMAKE_SOURCE_DIR}/external/inih/ini.c
    ${CMAKE_SOURCE_DIR}/external/inih/ini.h
    ${CMAKE_SOURCE_DIR}/external/inih/cpp/INIReader.h
    ${CMAKE_SOURCE_DIR}/external/inih/cpp/INIReader.cpp
)

add_library(inih-static STATIC ${inih_SOURCE_LIST})
add_library(inih::static ALIAS inih-static)
set_target_properties(inih-static PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:inih-static,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(inih-static PRIVATE INI_ALLOW_MULTILINE=0)
