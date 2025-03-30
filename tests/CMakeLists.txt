file(GLOB_RECURSE ALL_SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX ".*main\\.cpp$")

set(TEST_FILES
        parser/RedisParserTest.cpp
)

add_executable(redis_tests ${TEST_FILES} ${ALL_SOURCE_FILES})

target_include_directories(redis_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/parser
        ${CMAKE_SOURCE_DIR}/src/utils
        # ${CMAKE_SOURCE_DIR}/src/database
        # ${CMAKE_SOURCE_DIR}/src/network
        # ${CMAKE_SOURCE_DIR}/src/servers
)

target_link_libraries(redis_tests PRIVATE
        gtest
        gtest_main
        Threads::Threads
)

add_test(NAME RedisTests COMMAND redis_tests)