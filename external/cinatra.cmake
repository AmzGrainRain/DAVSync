# controls
set(BUILD_WITH_LIBCXX ON)
set(ENABLE_SANITIZER OFF)
set(ENABLE_SIMD "AVX2")
set(ENABLE_SSL ON)
set(ENABLE_GZIP ON)
set(ENABLE_BROTLI OFF)

message(STATUS "")
message(STATUS "========================== cinatra config ==========================")

message(STATUS "Build with libc++: ${BUILD_WITH_LIBCXX}")
if(BUILD_WITH_LIBCXX AND NOT WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# --------------------- Msvc
if (MSVC)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
	# Resolves C4737 complained by MSVC: C4737: Unable to perform required tail call. Performance may be degraded. "Release-Type only"
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/EHa>")
elseif(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")
endif()

# --------------------- Gcc
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
    #-ftree-slp-vectorize with coroutine cause link error. disable it util gcc fix.
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-tree-slp-vectorize")
endif()

# test asan
macro(check_asan _RESULT)
    include(CheckCXXSourceRuns)
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
    check_cxx_source_runs(
            [====[
int main()
{
  return 0;
}
]====]
            ${_RESULT}
    )
    unset(CMAKE_REQUIRED_FLAGS)
endmacro()

# address sanitizer
message(STATUS "Enable sanitizer: ${ENABLE_SANITIZER}")
if(ENABLE_SANITIZER AND NOT MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        check_asan(HAS_ASAN)
        if(HAS_ASAN)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
        else()
            message(WARNING "Sanitizer is no supported with current tool-chains.")
        endif()
    else()
        message(WARNING "Sanitizer supported only for CMAKE_BUILD_TYPE=Debug.")
    endif()
endif()

find_package(Threads REQUIRED)

# library properties
add_library(cinatra-static INTERFACE)
add_library(cinatra::static ALIAS cinatra-static)
set_target_properties(cinatra-static PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(cinatra-static SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/external/cinatra/include)

# simd
message(STATUS "Enable simd: ${ENABLE_SIMD}")
if (ENABLE_SIMD STREQUAL "AARCH64")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
		add_library(neon INTERFACE IMPORTED)
		target_compile_options(neon INTERFACE -march=armv8-a+fp+simd)
		target_link_libraries(cinatra-static INTERFACE neon)
	endif ()
elseif (ENABLE_SIMD STREQUAL "SSE42")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
		add_library(sse4_2 INTERFACE IMPORTED)
		if(MSVC)
			target_compile_options(sse4_2 INTERFACE /arch:SSE4.2)
		else()
			target_compile_options(sse4_2 INTERFACE -msse4.2)
		endif()
		target_link_libraries(cinatra-static INTERFACE sse4_2)
	endif ()
elseif (ENABLE_SIMD STREQUAL "AVX2")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
		add_library(avx2 INTERFACE IMPORTED)
		if(MSVC)
			target_compile_options(avx2 INTERFACE /arch:AVX2)
		else()
			target_compile_options(avx2 INTERFACE -mavx2)
		endif()
		target_link_libraries(cinatra-static INTERFACE avx2)
		set(CMAKE_CXX_FLAGS "-fpermissive")
	endif ()
endif ()

# fatures
message(STATUS "Enable OpenSSL: ${ENABLE_SSL}")
if (ENABLE_SSL)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(cinatra-static INTERFACE ${OPENSSL_LIBRARIES})
endif()

if (ENABLE_CLIENT_SSL)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(cinatra-static INTERFACE ${OPENSSL_LIBRARIES})
endif()

message(STATUS "Enable GZIP: ${ENABLE_GZIP}")
if (ENABLE_GZIP)
	find_package(ZLIB REQUIRED)
	target_link_libraries(cinatra-static INTERFACE ${ZLIB_LIBRARIES})
endif()

message(STATUS "Enable Brotli: ${ENABLE_BROTLI}")
if (ENABLE_BROTLI)
	include_directories(${BROTLI_INCLUDE_DIRS})
	target_link_libraries(cinatra-static INTERFACE ${BROTLI_LIBRARIES})
endif()

message(STATUS "====================================================================")
message(STATUS "")
