
# cpr configuration.  Ideally we'd just do this via add_subdirectory, but cpr's cmake requires
# 3.15+, and we target lower than that (and this is fairly simple to build).

if(NOT BUILD_STATIC_DEPS)
  find_package(CURL REQUIRED COMPONENTS HTTP HTTPS SSL)

  # CURL::libcurl wasn't added to FindCURL until cmake 3.12, so add it if necessary
  if (CMAKE_VERSION VERSION_LESS 3.12 AND NOT TARGET CURL::libcurl)
    add_library(libcurl UNKNOWN IMPORTED GLOBAL)
    set_target_properties(libcurl PROPERTIES
      IMPORTED_LOCATION ${CURL_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
    add_library(CURL_libcurl INTERFACE)
    target_link_libraries(CURL_libcurl INTERFACE libcurl)
    add_library(CURL::libcurl ALIAS CURL_libcurl)
  endif()
endif()
math(EXPR cpr_VERSION_NUM "1 * 0x10000 + 7 * 0x100 + 0" OUTPUT_FORMAT HEXADECIMAL)

configure_file(cpr/cmake/cprver.h.in ${CMAKE_BINARY_DIR}/cpr_generated/cpr/cprver.h)

file(GLOB cpr_sources ${conf_depends} cpr/cpr/*.cpp)

add_library(cpr STATIC EXCLUDE_FROM_ALL ${cpr_sources})
target_link_libraries(cpr PUBLIC CURL::libcurl)
target_include_directories(cpr PUBLIC cpr/include ${CMAKE_BINARY_DIR}/cpr_generated)
target_compile_definitions(cpr PUBLIC CPR_CURL_NOSIGNAL)
add_library(cpr::cpr ALIAS cpr)



include(ExternalProject)
ExternalProject_Add(libsodium-internal_ext
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libsodium-internal
    BUILD_IN_SOURCE TRUE
    CONFIGURE_COMMAND ./autogen.sh && ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/libsodium-internal
        --disable-shared --enable-static --with-pic "CC=${CMAKE_C_COMPILER}"
    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/libsodium-internal/lib/libsodium.a
        ${CMAKE_CURRENT_BINARY_DIR}/libsodium-internal/include/sodium.h
)
add_library(libsodium-internal STATIC IMPORTED GLOBAL)
add_dependencies(libsodium-internal libsodium-internal_ext)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/libsodium-internal/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libsodium-internal/include)

set_target_properties(libsodium-internal PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libsodium-internal/lib/libsodium.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libsodium-internal/include
)
