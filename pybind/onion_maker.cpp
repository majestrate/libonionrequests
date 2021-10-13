#include "common.hpp"
#include <onionreq/onion_maker.hpp>

namespace onionreq
{

  void
  OnionMaker_Init(py::module & mod)
  {

    auto submod = mod.def_submodule("onionmaker");
    py::class_<OnionMaker_Base, std::unique_ptr<OnionMaker_Base>>(submod, "OnionMakerBase")
      .def("make_onion", [](const std::unique_ptr<OnionMaker_Base> & self, std::string plaintext, const OnionPath & path) { return self->MakeOnionFromHops(std::move(plaintext), path); });

    submod.def("all_xchacha20", []() {
      return std::unique_ptr<OnionMaker_Base>{OnionMaker(all_xchacha20_hops{})}; });
    submod.def("all_aesgcm", []() {
      return std::unique_ptr<OnionMaker_Base>{OnionMaker(all_aesgcm_hops{})}; });
  } 
  
} 
