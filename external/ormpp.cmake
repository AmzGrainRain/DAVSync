set(ormpp-CXX_STANDARD 17)
set(ENABLE_SQLITE3 ON)

add_library(ormpp-headers INTERFACE)
add_library(ormpp::headers ALIAS ormpp-headers)
set_target_properties(ormpp-headers PROPERTIES
    CXX_STANDARD ${ormpp-CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(ormpp-headers SYSTEM INTERFACE
    ormpp
    ormpp/ormpp
    ormpp/thirdparty/sqlite3
)

add_library(sqlite3-static STATIC
    ormpp/thirdparty/sqlite3/sqlite3.c
    ormpp/thirdparty/sqlite3/sqlite3.h
    ormpp/thirdparty/sqlite3/sqlite3ext.h
)
add_library(sqlite3::static ALIAS sqlite3-static)
target_include_directories(sqlite3-static SYSTEM INTERFACE ormpp/thirdparty/sqlite3)
target_compile_definitions(ormpp-headers INTERFACE -DORMPP_ENABLE_SQLITE3)
target_link_libraries(ormpp-headers INTERFACE sqlite3::static)

message(STATUS "")
message(STATUS "")
message(STATUS "=========================== ormpp config ===========================")
message(STATUS "CXX Standard: ${ormpp-CXX_STANDARD}")
message(STATUS "Target: ormpp::headers sqlite3::static")
get_target_property(ormpp-headers_INCLUDES ormpp::headers INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Includes: ${ormpp-headers_INCLUDES}")
