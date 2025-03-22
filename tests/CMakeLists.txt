project(tests)
enable_testing()
find_package(GTest CONFIG REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/src)
link_libraries(
    server_static
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

add_executable(test_ConfigReader test_ConfigReader.cpp)
add_test(NAME Test_ConfigReader COMMAND test_ConfigReader)

# add_executable(test_ormpp test_ormpp.cpp)
# target_link_libraries(test_ormpp PUBLIC ormpp::headers)
# add_test(test_ormpp COMMAND test_ormpp)

# add_executable(test_redis test_redis.cpp)
# target_link_libraries(test_redis PUBLIC hiredis::hiredis)
# add_test(test_redis COMMAND test_redis)

# add_executable(test_file_lock test_file_lock.cpp)
# add_test(test_file_lock COMMAND test_file_lock)

# add_executable(test_xml test_xml.cpp)
# target_link_libraries(test_xml PUBLIC pugixml::static)

# add_executable(test_json test_json.cpp)
# target_link_libraries(test_json PUBLIC iguana::headers)
