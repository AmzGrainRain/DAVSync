set(cinatra-CXX_STANDARD 20)
set(ENABLE_SIMD "AVX2")
set(ENABLE_SSL ON)
set(ENABLE_GZIP ON)

add_library(cinatra-headers INTERFACE)
add_library(cinatra::headers ALIAS cinatra-headers)
set_target_properties(cinatra-headers PROPERTIES
    CXX_STANDARD ${cinatra-CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(cinatra-headers SYSTEM INTERFACE cinatra/include)

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

find_package(Threads REQUIRED)

# simd
if (ENABLE_SIMD STREQUAL "AARCH64")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
		add_library(neon INTERFACE IMPORTED)
		target_compile_options(neon INTERFACE -march=armv8-a+fp+simd)
		target_link_libraries(cinatra-headers INTERFACE neon)
	endif ()
elseif (ENABLE_SIMD STREQUAL "SSE42")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
		add_library(sse4_2 INTERFACE IMPORTED)
		if(MSVC)
			target_compile_options(sse4_2 INTERFACE /arch:SSE4.2)
		else()
			target_compile_options(sse4_2 INTERFACE -msse4.2)
		endif()
		target_link_libraries(cinatra-headers INTERFACE sse4_2)
	endif ()
elseif (ENABLE_SIMD STREQUAL "AVX2")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
		add_library(avx2 INTERFACE IMPORTED)
		if(MSVC)
			target_compile_options(avx2 INTERFACE /arch:AVX2)
		else()
			target_compile_options(avx2 INTERFACE -mavx2)
		endif()
		target_link_libraries(cinatra-headers INTERFACE avx2)
		set(CMAKE_CXX_FLAGS "-fpermissive")
	endif ()
endif ()

# fatures
if (ENABLE_SSL)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(cinatra-headers INTERFACE ${OPENSSL_LIBRARIES})
endif()

if (ENABLE_GZIP)
	find_package(ZLIB REQUIRED)
	target_link_libraries(cinatra-headers INTERFACE ${ZLIB_LIBRARIES})
endif()

message(STATUS "")
message(STATUS "")
message(STATUS "========================== cinatra config ==========================")
message(STATUS "CXX Standard: ${cinatra-CXX_STANDARD}")
message(STATUS "Enable simd: ${ENABLE_SIMD}")
message(STATUS "Enable OpenSSL: ${ENABLE_SSL}")
message(STATUS "Enable GZIP: ${ENABLE_GZIP}")
message(STATUS "Target: cinatra::headers")
get_target_property(cinatra-headers_INCLUDES cinatra::headers INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Includes: ${cinatra-headers_INCLUDES}")
