add_subdirectory(${CMAKE_SOURCE_DIR}/external/PicoSHA2)
set_target_properties(picosha2 PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:picosha2,INTERFACE_INCLUDE_DIRECTORIES>)

add_library(picosha2::static ALIAS picosha2)
