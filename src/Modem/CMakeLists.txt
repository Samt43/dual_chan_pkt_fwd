cmake_minimum_required(VERSION 3.7)
enable_language(CXX)
find_package (Threads)

SET(STUB "1")

if(NOT BUILD_TYPE STREQUAL "STUB")
   find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
   find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)
   set(FolderImpl "ImplLoraModem")
else()
   set(FolderImpl "StubLoraModem")
endif()

project(LoraModem)
include_directories(${CMAKE_SOURCE_DIR})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/${FolderImpl}/*.cpp)

add_library(LoraModem STATIC ${SRC_FILES})

if(NOT BUILD_TYPE STREQUAL "STUB")
   target_link_libraries(LoraModem Threads::Threads)
   target_link_libraries(LoraModem ${WIRINGPI_LIBRARIES})
endif()

