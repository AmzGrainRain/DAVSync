project(server)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/*.hpp
    ${PROJECT_SOURCE_DIR}/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC INIH_LIB pugixml::static)

# fot cinatra
target_compile_definitions(${PROJECT_NAME} PUBLIC ASYNC_SIMPLE_HAS_NOT_AIO)

if (ENABLE_SSL)
	find_package(OpenSSL)
	target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} pthread -ldl)
endif()

if (ENABLE_CLIENT_SSL)
	find_package(OpenSSL)
	target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES} pthread -ldl)
endif()

if (ENABLE_GZIP)
	target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
endif()

if (ENABLE_BROTLI)
	include_directories(${BROTLI_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${BROTLI_LIBRARIES})
endif()

if (ENABLE_SIMD STREQUAL "AARCH64")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
		add_library(neon INTERFACE IMPORTED)
		target_compile_options(neon INTERFACE -march=armv8-a+fp+simd)
		target_link_libraries(${PROJECT_NAME} neon)
	endif ()
elseif (ENABLE_SIMD STREQUAL "SSE42")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
		add_library(sse4_2 INTERFACE IMPORTED)
		if(MSVC)
			target_compile_options(sse4_2 INTERFACE /arch:SSE4.2)
		else()
			target_compile_options(sse4_2 INTERFACE -msse4.2)
		endif()
		target_link_libraries(${PROJECT_NAME} sse4_2)
	endif ()
elseif (ENABLE_SIMD STREQUAL "AVX2")
	if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
		add_library(avx2 INTERFACE IMPORTED)
		if(MSVC)
			target_compile_options(avx2 INTERFACE /arch:AVX2)
		else()
			target_compile_options(avx2 INTERFACE -mavx2)
		endif()
		target_link_libraries(${PROJECT_NAME} avx2)
		set(CMAKE_CXX_FLAGS "-fpermissive")
	endif ()
endif ()

# for pugixml
# target_compile_definitions(${PROJECT_NAME} PUBLIC)

# for tests
add_library(${PROJECT_NAME}_LIB STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}_LIB PUBLIC INIH_LIB)
target_include_directories(${PROJECT_NAME}_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
