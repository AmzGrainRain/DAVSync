add_library(ormpp-static INTERFACE)
add_library(ormpp::static ALIAS ormpp-static)
set_target_properties(ormpp-static PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(ormpp-static SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/external/ormpp/ormpp)
target_include_directories(ormpp-static SYSTEM PUBLIC ${PGSQL_INCLUDE_DIR})

find_package(libpqxx CONFIG REQUIRED)
target_link_libraries(ormpp-static INTERFACE libpqxx::pqxx)

if (NOT WIN32)
    target_compile_options(ormpp-static INTERFACE -pthread -ldl)
endif()
