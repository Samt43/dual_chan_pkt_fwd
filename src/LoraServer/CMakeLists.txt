cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 14)
enable_language(CXX)
find_package (Threads)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2")

project(LoraServer)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(../)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/LoraServerImpl/*.cpp)


add_executable(LoraServer ${SRC_FILES})
target_link_libraries(LoraServer LoraModem)

