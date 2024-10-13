project(tests)

add_executable(test_ormpp test_ormpp.cpp)
target_link_libraries(test_ormpp PUBLIC ormpp::headers)
add_test(test_ormpp COMMAND test_ormpp)

add_executable(test_redis test_redis.cpp)
target_link_libraries(test_redis PUBLIC hiredis::hiredis)
add_test(test_redis COMMAND test_redis)

add_executable(test_file_lock test_file_lock.cpp)
add_test(test_file_lock COMMAND test_file_lock)

add_executable(test_xml test_xml.cpp)
target_link_libraries(test_xml PUBLIC pugixml::static)
