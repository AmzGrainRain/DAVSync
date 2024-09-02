add_library(ormpp-static INTERFACE)
add_library(ormpp::static ALIAS ormpp-static)
target_include_directories(ormpp-static SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/external/ormpp/ormpp)
target_include_directories(ormpp-static SYSTEM PUBLIC ${PGSQL_INCLUDE_DIR})

find_package(libpqxx CONFIG REQUIRED)
target_link_libraries(ormpp-static INTERFACE libpqxx::pqxx)

if (NOT WIN32)
    target_compile_options(ormpp-static INTERFACE -pthread -ldl)
endif()
