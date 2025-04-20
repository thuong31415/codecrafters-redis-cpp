cmake_minimum_required(VERSION 3.14)

project(redis-starter-cpp)

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Multithreading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Boost
find_package(Boost REQUIRED)

# Project source files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

# Add executable
add_executable(server ${SOURCE_FILES})
target_link_libraries(server PRIVATE Threads::Threads Boost::boost)

# Include directories
target_include_directories(server PRIVATE
        src/parser
        src/utils
        src/database
        src/network
        src/servers
)