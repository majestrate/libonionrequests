find_package(pybind11 REQUIRED)

pybind11_add_module(pyonionreq MODULE
  module.cpp
  onion_maker.cpp
  common.cpp
  consensus.cpp
  transport.cpp
  junk.cpp
  xed25519.cpp
)

target_link_libraries(pyonionreq PUBLIC onionreq)
