OPTION(ENABLE_SANITIZER OFF)

include(${CMAKE_SOURCE_DIR}/external/cinatra/cmake/build.cmake)
include(${CMAKE_SOURCE_DIR}/external/cinatra/cmake/develop.cmake)

include_directories(${CMAKE_SOURCE_DIR}/external/cinatra)
include_directories(${CMAKE_SOURCE_DIR}/external/cinatra/include)

#the thread library of the system.
find_package(Threads REQUIRED)
