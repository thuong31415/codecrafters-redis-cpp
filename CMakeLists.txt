cmake_minimum_required(VERSION 3.14)

project(redis-starter-cpp)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Multithreading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Project source files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

# Add executable
add_executable(server ${SOURCE_FILES})
target_link_libraries(server PRIVATE Threads::Threads)


# Include directories
target_include_directories(server PRIVATE
        src/parser
        src/utils
        src/database
        src/network
        src/servers
)