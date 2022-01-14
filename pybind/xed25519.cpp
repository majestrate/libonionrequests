#include "common.hpp"
#include <onionreq/signal-xed25519.hpp>

namespace onionreq
{
  using xed25519::ustring_view;

  ustring_view
  uview(py::bytes& in)
  {
    char* ptr;
    ssize_t sz;
    PyBytes_AsStringAndSize(in.ptr(), &ptr, &sz);
    return ustring_view{reinterpret_cast<unsigned char*>(ptr), static_cast<size_t>(sz)};
  }

  void
  XEd25519_Init(py::module& mod)
  {
    using namespace pybind11::literals;
    auto submod = mod.def_submodule("xed25519");
    submod.def("sign", [](py::bytes curve25519_privkey, py::bytes msg) {
        auto sig = xed25519::sign(uview(curve25519_privkey), uview(msg));
        return py::bytes(reinterpret_cast<const char*>(sig.data()), sig.size());
      },
      "curve25519_privkey"_a, "msg"_a,
      "Constructs an XEd25519 signature of `msg` (max 256 bytes) using the given private key");

    submod.def("pubkey", [](py::bytes curve25519_pubkey) {
        auto edpk = xed25519::pubkey(uview(curve25519_pubkey));
        return py::bytes(reinterpret_cast<const char*>(edpk.data()), edpk.size());
      },
      "curve25519_pubkey"_a,
      "Returns the derived Ed25519 pubkey from a curve25519 pubkey");

    submod.def("verify", [](py::bytes signature, py::bytes curve25519_pubkey, py::bytes msg) {
            return xed25519::verify(uview(signature), uview(curve25519_pubkey), uview(msg));
        },
        "signature"_a, "curve25519_pubkey"_a, "msg"_a,
        "Verifies the XEd25519 signature `signature` of `msg` allegedly signed by `curve25519_pubkey`.");
  }
}  // namespace onionreq
