#include "common.hpp"
#include <onionreq/onion_maker.hpp>

namespace onionreq
{
  struct PyOnionMaker
  {
    std::unique_ptr<OnionMaker_Base> impl;
    explicit PyOnionMaker(OnionMaker_Base* _impl) : impl{_impl}
    {}
  };

  void
  OnionMaker_Init(py::module& mod)
  {
    auto submod = mod.def_submodule("onionmaker");
    py::class_<PyOnionMaker>(submod, "OnionMakerBase")
        .def(
            "make_onion",
            [](const PyOnionMaker& self, std::string plaintext, const OnionPath& path)
                -> OnionPayload { return self.impl->MakeOnion(std::move(plaintext), path); });

    submod.def("all_xchacha20", []() { return PyOnionMaker{OnionMaker(all_xchacha20_hops{})}; });
    submod.def("all_aesgcm", []() { return PyOnionMaker{OnionMaker(all_aesgcm_hops{})}; });
  }

}  // namespace onionreq
