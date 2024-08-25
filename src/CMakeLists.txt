project(server)

# executable
file(GLOB_RECURSE SOURCE_LIST
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/*.hpp
    ${PROJECT_SOURCE_DIR}/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
    cinatra::static
    inih::static
    pugixml::static
    picosha2::static
)

# generate static library
add_library(${PROJECT_NAME}-static STATIC ${SOURCE_LIST})
target_include_directories(${PROJECT_NAME}-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME}-static PUBLIC
    cinatra-static
	inih::static
	pugixml::static
    picosha2::static
)
add_library(${PROJECT_NAME}::static ALIAS ${PROJECT_NAME}-static)
