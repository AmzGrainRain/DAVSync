cmake_minimum_required(VERSION 3.6)

project(DavSYNC LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

include(external/inih.cmake)
include(external/cinatra.cmake)
include(external/pugixml.cmake)

include_directories(external)
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
