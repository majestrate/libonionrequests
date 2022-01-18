cmake_minimum_required(VERSION 3.10)  # bionic's cmake version

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Has to be set before `project()`, and ignored on non-macos:
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12 CACHE STRING "macOS deployment target (Apple clang only)")

set(LANGS CXX C)

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
    VERSION 0.0.2
    DESCRIPTION "libonionrequests - onionrequests library in C++ with some nice wrappers that dont suck that much"
    LANGUAGES ${LANGS})


find_package(PkgConfig REQUIRED)

include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(WITH_PYBIND "build pybind module" OFF)
option(WITH_NODEJS "build nodejs native bindings" OFF)
option(WITH_JNI "build jni bindings" OFF)

pkg_check_modules(OXENMQ liboxenmq IMPORTED_TARGET REQUIRED)
pkg_check_modules(SODIUM libsodium IMPORTED_TARGET REQUIRED)
pkg_check_modules(CRYPTO libcrypto IMPORTED_TARGET REQUIRED)
pkg_check_modules(OXENC liboxenc>=1.0.1 IMPORTED_TARGET REQUIRED)
pkg_check_modules(SPDLOG spdlog IMPORTED_TARGET REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(external/libsignal-protocol-c/src/curve25519)


add_library(onionreq
  onionreq/channel_encryption.cpp
  onionreq/consensus.cpp
  onionreq/junk.cpp
  onionreq/key_types.cpp
  onionreq/signature.cpp
  onionreq/sogs_info.cpp
  onionreq/snode_info.cpp
  onionreq/transport.cpp
  onionreq/onion_maker.cpp
  onionreq/signal-xed25519.cpp)

target_compile_options(onionreq PUBLIC -fPIC)
target_link_libraries(onionreq PUBLIC PkgConfig::SPDLOG PkgConfig::OXENMQ PkgConfig::SODIUM PkgConfig::CRYPTO nlohmann_json::nlohmann_json)
target_include_directories(onionreq PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set(XED_INC "")
foreach(p "" "/additions" "/nacl_includes")
    list(APPEND XEDDSA_INC "${CMAKE_CURRENT_SOURCE_DIR}/external/libsignal-protocol-c/src/curve25519/ed25519${p}")
endforeach()
set_source_files_properties(onionreq/signal-xed25519.cpp
    PROPERTIES INCLUDE_DIRECTORIES "${XEDDSA_INC}")
target_link_libraries(onionreq PRIVATE curve25519)


add_subdirectory(docs)

if(WITH_PYBIND)
  add_subdirectory(pybind)
endif()

if(WITH_NODEJS)
  add_subdirectory(nodejs)
endif()

if(WITH_JNI)
  add_subdirectory(jni)
endif()

option(WITH_TESTS "build tests" ON)

if(WITH_TESTS)
  add_subdirectory(external)
  add_subdirectory(tests)
endif()
