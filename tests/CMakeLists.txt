add_executable(aipp_test main_test.cpp)
target_link_libraries(aipp_test PRIVATE GTest::gtest GTest::gtest_main)
target_include_directories(aipp_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME aipp_test COMMAND aipp_test)
