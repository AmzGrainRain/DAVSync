set(ormpp-CXX_STANDARD 17)
set(ENABLE_SQLITE3 ON)

add_library(ormpp-headers INTERFACE)
add_library(ormpp::headers ALIAS ormpp-headers)
set_target_properties(ormpp-headers PROPERTIES
    CXX_STANDARD ${ormpp-CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(ormpp-headers SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/external/ormpp
    ${CMAKE_SOURCE_DIR}/external/ormpp/ormpp
)

if(ENABLE_SQLITE3)
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    target_link_libraries(ormpp-headers INTERFACE unofficial::sqlite3::sqlite3)
    add_definitions(-DORMPP_ENABLE_SQLITE3)
endif()

message(STATUS "")
message(STATUS "")
message(STATUS "=========================== ormpp config ===========================")
message(STATUS "CXX Standard: ${ormpp-CXX_STANDARD}")
message(STATUS "Enable SQLite3: ${ENABLE_SQLITE3}")
message(STATUS "Target: ormpp::headers")
get_target_property(ormpp-headers_INCLUDES ormpp::headers INTERFACE_INCLUDE_DIRECTORIES )
message(STATUS "Includes: ${ormpp-headers_INCLUDES}")
