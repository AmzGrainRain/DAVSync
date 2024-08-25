project(tests)

# test for config_reader.cpp
add_executable(test_config_reader config_reader/config_reader.cpp)
target_link_libraries(test_config_reader PUBLIC server::static)
add_test(NAME test_config_reader COMMAND test_config_reader)
