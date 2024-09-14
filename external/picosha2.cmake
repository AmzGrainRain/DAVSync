add_subdirectory(${CMAKE_SOURCE_DIR}/external/PicoSHA2)
add_library(picosha2::static ALIAS picosha2)
set_target_properties(picosha2 PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:picosha2,INTERFACE_INCLUDE_DIRECTORIES>
)
