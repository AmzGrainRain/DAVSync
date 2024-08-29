add_subdirectory(${CMAKE_SOURCE_DIR}/external/pugixml)
include_directories(${CMAKE_SOURCE_DIR}/external/pugixml/src)
set_target_properties(pugixml-static PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:pugixml-static,INTERFACE_INCLUDE_DIRECTORIES>)
