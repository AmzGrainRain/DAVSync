set(inih-CXX_STANDARD 17)
file(GLOB_RECURSE inih_SOURCE_LIST
    inih/ini.c
    inih/ini.h
    inih/cpp/INIReader.h
    inih/cpp/INIReader.cpp
)

add_library(inih-static STATIC ${inih_SOURCE_LIST})
add_library(inih::static ALIAS inih-static)
set_target_properties(inih-static PROPERTIES
    CXX_STANDARD ${inih-CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:inih-static,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(inih-static PRIVATE INI_ALLOW_MULTILINE=0)

message(STATUS "")
message(STATUS "")
message(STATUS "=========================== inih config ============================")
message(STATUS "CXX Standard: ${inih-CXX_STANDARD}")
message(STATUS "Target: inih::static")
get_target_property(inih-static_INCLUDES inih::static INCLUDE_DIRECTORIES)
message(STATUS "Includes: ${inih-static_INCLUDES}")
