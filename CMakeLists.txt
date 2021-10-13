cmake_minimum_required(VERSION 3.10)  # bionic's cmake version

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Has to be set before `project()`, and ignored on non-macos:
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12 CACHE STRING "macOS deployment target (Apple clang only)")

set(LANGS CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  foreach(lang ${LANGS})
    if(NOT DEFINED CMAKE_${lang}_COMPILER_LAUNCHER AND NOT CMAKE_${lang}_COMPILER MATCHES ".*/ccache")
      message(STATUS "Enabling ccache for ${lang}")
      set(CMAKE_${lang}_COMPILER_LAUNCHER ${CCACHE_PROGRAM} CACHE STRING "")
    endif()
  endforeach()
endif()

project(onionrequests
    VERSION 0.0.1
    DESCRIPTION "libonionrequests - onionrequests library in C++ with some nice wrappers that dont suck that much"
    LANGUAGES ${LANGS})


find_package(PkgConfig REQUIRED)

include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(WITH_PYBIND "build pybind module" ON)
option(WITH_NODEJS "build nodejs module" ON)


add_library(onionreq
  onionreq/consensus.cpp
  onionreq/snode_info.cpp
  onionreq/transport.cpp
  onionreq/onion_maker.cpp)

if(WITH_PYBIND)
  add_subdirectroy(pybind)
endif()

if(WITH_NODEJS)
  add_subdirectory(nodejs)
endif()
