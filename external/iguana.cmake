set(iguana-CXX_STANDARD 17)

add_library(iguana-headers INTERFACE)
add_library(iguana::headers ALIAS iguana-headers)
set_target_properties(iguana-headers PROPERTIES
        CXX_STANDARD ${iguana-CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
)
target_include_directories(iguana-headers SYSTEM INTERFACE iguana)

if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
else()
    if(NOT LINKLIBC++)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g ")
        message(STATUS "link libstdc++")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g -stdlib=libc++")
        message(STATUS "link libc++")
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(MSVC)

set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

add_definitions(-DTHROW_UNKNOWN_KEY)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")

option(ENABLE_SEQUENTIAL_PARSE "parse json sequential more efficient if the json fields sequences are the same with struct fields" OFF)
if (${ENABLE_SEQUENTIAL_PARSE})
    add_definitions(-DSEQUENTIAL_PARSE)
endif ()

option(HAS_RAPIDJSON "import rapidjson" OFF)
if (${HAS_RAPIDJSON})
    add_definitions(-DHAS_RAPIDJSON)
    include_directories(../rapidjson/include)
    add_definitions(-DRAPIDJSON_HAS_STDSTRING)
endif()

option(HAS_RAPIDYAML "import rapidyaml" OFF)
if (${HAS_RAPIDYAML})
    add_definitions(-DHAS_RAPIDYAML)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    link_libraries(stdc++fs)
endif()

find_package(Protobuf QUIET)
if(Protobuf_FOUND)
    add_definitions(-DSTRUCT_PB_WITH_PROTO)
    find_package(Protobuf REQUIRED)
    if(Protobuf_FOUND)
        message(STATUS "Found Protobuf: ${Protobuf_VERSION}")
    else()
        message(STATUS "Protobuf not found")
    endif()
endif()
