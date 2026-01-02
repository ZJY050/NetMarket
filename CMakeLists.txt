cmake_minimum_required(VERSION 3.10)
project(NetMarket LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Define a macro DATA_DIR with absolute path to data directory so runtime file IO is deterministic.
add_definitions(-DDATA_DIR=\"${CMAKE_SOURCE_DIR}/data/\")

file(GLOB SRCS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(NetMarket ${SRCS})
target_link_libraries(NetMarket stdc++fs)

