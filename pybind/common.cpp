#include "common.hpp"
#include <onionreq/onion.hpp>

namespace onionreq
{
  void
  Common_Init(py::module& mod)
  {
    py::class_<SNodeInfo>(mod, "SNodeInfo");

    py::class_<SOGSInfo>(mod, "SOGSInfo");

    py::class_<OnionPath>(mod, "OnionPath")
        .def_readwrite("hops", &OnionPath::hops)
        .def_readwrite("remote", &OnionPath::remote);

    py::class_<OnionPayload>(mod, "OnionPayload")
        .def_readonly("ciphertext", &OnionPayload::ciphertext)
        .def_readonly("path", &OnionPayload::path);
  }
}  // namespace onionreq
