pybind11_add_module(pyonionreq MODULE
  module.cpp
  onion_maker.cpp
  common.cpp
  consensus.cpp
)

target_link_libraries(pyonionreq PUBLIC onionreq)
