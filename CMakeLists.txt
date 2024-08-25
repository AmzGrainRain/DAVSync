cmake_minimum_required(VERSION 3.6)

project(DavSYNC LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_definitions(
	-D_SCL_SECURE_NO_WARNINGS
	-D_CRT_SECURE_NO_WARNINGS
	-D_WIN32_WINNT=0x0A00
)

# external
include_directories(external)
include(external/inih.cmake)
include(external/cinatra.cmake)
include(external/pugixml.cmake)
include(external/picosha2.cmake)

# src
add_subdirectory(src)

# tests
enable_testing()
add_subdirectory(tests)

# warning control
file(GLOB_RECURSE SOURCE_LIST
    src/*.h
    src/*.hpp
    src/*.cxx
    src/*.cpp
)

file(GLOB_RECURSE EXTERNAL_SOURCE_LIST
    external/*.h
    external/*.ipp
    external/*.hpp
    external/*.c
    external/*.cc
    external/*.cxx
    external/*.cpp
)

if (MSVC)
    set_source_files_properties(${SOURCE_LIST} PROPERTIES COMPILE_FLAGS "/W4 /WX")
    set_source_files_properties(${EXTERNAL_SOURCE_LIST} PROPERTIES COMPILE_FLAGS "/w")
else()
    set_source_files_properties(${SOURCE_LIST} PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Werror")
    set_source_files_properties(${EXTERNAL_SOURCE_LIST} PROPERTIES COMPILE_FLAGS "-w")
endif()
