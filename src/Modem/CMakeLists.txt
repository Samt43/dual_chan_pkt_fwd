cmake_minimum_required(VERSION 3.7)
set (CMAKE_CXX_STANDARD 14)
enable_language(CXX)
find_package (Threads)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2")

if(NOT BUILD_TYPE STREQUAL "STUB")
   find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
   find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)
   set(FolderImpl "ImplLoraModem")
else()
   set(FolderImpl "StubLoraModem")
endif()

project(LoraModem)
include_directories(${PROJECT_SOURCE_DIR})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/${FolderImpl}/*.cpp)

add_library(LoraModem STATIC ${SRC_FILES})

if(NOT BUILD_TYPE STREQUAL "STUB")
   target_link_libraries(LoraModem Threads::Threads)
   target_link_libraries(LoraModem ${WIRINGPI_LIBRARIES})
endif()

file(GLOB SRC_TEST_FILES ${PROJECT_SOURCE_DIR}/Test/*.cpp)
add_executable(testModem ${PROJECT_SOURCE_DIR}/Test/test.cpp)
target_link_libraries(testModem LoraModem)

