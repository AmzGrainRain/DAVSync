add_subdirectory(${CMAKE_SOURCE_DIR}/external/pugixml)
include_directories(${CMAKE_SOURCE_DIR}/external/pugixml/src)
set_target_properties(pugixml-static PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:pugixml-static,INTERFACE_INCLUDE_DIRECTORIES>
)
