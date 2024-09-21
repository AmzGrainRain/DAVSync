cmake_minimum_required(VERSION 3.6)

project(DavSYNC LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(BUILD_WITH_LIBCXX ON)

add_definitions(
    -D_SCL_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
)
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# vcpkg
find_package(spdlog CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)

# external
include_directories(external)
include(external/inih.cmake)
include(external/cinatra.cmake)
include(external/ormpp.cmake)
include(external/pugixml.cmake)
include(external/picosha2.cmake)

# src
add_subdirectory(src)

# tests
enable_testing()
add_subdirectory(tests)

# build with libc++
if(BUILD_WITH_LIBCXX AND NOT WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

message(STATUS "")
message(STATUS "========================== server config ===========================")
message(STATUS "Build with libc++: ${BUILD_WITH_LIBCXX}")
message(STATUS "CXX Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Toolchain: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Build: ${CMAKE_BUILD_TYPE}")
