project(tests)

add_executable(test_ormpp test_ormpp.cpp)
add_test(test_ormpp COMMAND test_ormpp)

target_link_libraries(test_ormpp PUBLIC ormpp::headers)
