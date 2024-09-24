set(pugixml-CXX_STANDARD 11)

add_subdirectory(pugixml)
set_target_properties(pugixml-static PROPERTIES
    CXX_STANDARD ${pugixml-CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:pugixml-static,INTERFACE_INCLUDE_DIRECTORIES>
)

message(STATUS "")
message(STATUS "")
message(STATUS "=========================== pugixml config ============================")
message(STATUS "CXX Standard: ${pugixml-CXX_STANDARD}")
message(STATUS "Target: pugixml::static")
get_target_property(pugixml-static_INCLUDES pugixml::static INCLUDE_DIRECTORIES)
message(STATUS "Includes: ${pugixml-static_INCLUDES}")
