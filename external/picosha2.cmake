set(picosha2-CXX_STANDARD 17)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/PicoSHA2)
add_library(picosha2::static ALIAS picosha2)
set_target_properties(picosha2 PROPERTIES
    CXX_STANDARD ${picosha2-CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:picosha2,INTERFACE_INCLUDE_DIRECTORIES>
)

message(STATUS "")
message(STATUS "")
message(STATUS "=========================== picosha2 config ===========================")
message(STATUS "CXX Standard: ${picosha2-CXX_STANDARD}")
message(STATUS "Target: picosha2::static")
get_target_property(picosha2-static_INCLUDES picosha2::static INCLUDE_DIRECTORIES)
message(STATUS "Includes: ${picosha2-static_INCLUDES}")
