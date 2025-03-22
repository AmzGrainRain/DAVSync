project(server)

file(GLOB_RECURSE SOURCE_LIST
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/*.hpp
    ${PROJECT_SOURCE_DIR}/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/ormpp/thirdparty/sqlite3
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::uuid
    inih::static
    cinatra::headers
    ormpp::headers
    hiredis::hiredis
    pugixml::static
    picosha2::static
)

# static library for testing
add_library(${PROJECT_NAME}_static STATIC ${SOURCE_LIST})
target_include_directories(${PROJECT_NAME}_static PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/ormpp/thirdparty/sqlite3
)
target_link_libraries(${PROJECT_NAME}_static PRIVATE
    Boost::uuid
    inih::static
    cinatra::headers
    ormpp::headers
    hiredis::hiredis
    pugixml::static
    picosha2::static
)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} INTERFACE "/W4 /WX /experimental:module")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} INTERFACE "-Wall -Wextra -Werror -fmodules-ts")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} INTERFACE "-Wall -Wextra -Werror -fmodules-ts")
endif()
