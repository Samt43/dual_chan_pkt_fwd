cmake_minimum_required(VERSION 3.7)
enable_language(CXX)
find_package (Threads)

find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)

project(dual_chan_pkt_fwd)
include_directories(${CMAKE_SOURCE_DIR}/src)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(dual_chan_pkt_fwd ${SRC_FILES})
target_link_libraries(dual_chan_pkt_fwd Threads::Threads)
target_link_libraries(dual_chan_pkt_fwd ${WIRINGPI_LIBRARIES})

