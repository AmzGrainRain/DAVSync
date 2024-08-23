cmake_minimum_required(VERSION 3.6)

project(DavSYNC LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
# set(BUILD_STATIC_LIBS ON)

include(external/inih.cmake)
include(external/cinatra.cmake)
include(external/pugixml.cmake)

add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
