project(server)

# executable
file(GLOB_RECURSE SOURCE_LIST
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/*.hpp
    ${PROJECT_SOURCE_DIR}/*.cpp
)
add_executable(${PROJECT_NAME} ${SOURCE_LIST})
target_link_libraries(${PROJECT_NAME} PUBLIC
	inih::static
	pugixml::static
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

# for cinatra
include(cmake/cinatra.cmake)

# for tests
add_library(${PROJECT_NAME}-static STATIC ${SOURCE_LIST})
target_link_libraries(${PROJECT_NAME}-static PUBLIC
	inih::static
	pugixml::static
)
add_library(${PROJECT_NAME}::static ALIAS ${PROJECT_NAME}-static)
