cmake_minimum_required(VERSION 3.7.2)

# You can tweak some common (for all subprojects) stuff here. For example:

set(STUB ON)


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# When done tweaking common stuff, configure the components (subprojects).
# NOTE: The order matters! The most independent ones should go first.
add_subdirectory(Modem) # B is a static library (depends on Boost)
add_subdirectory(LoraServer) # C is a shared library (depends on B and external XXX)

