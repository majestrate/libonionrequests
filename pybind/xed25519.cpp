#include "common.hpp"
#include <onionreq/signal-xed25519.hpp>
#include <stdexcept>

using namespace std::literals;

namespace onionreq
{
  using xed25519::ustring_view;

  ustring_view
  uview(py::bytes& in, size_t required_size = 0, const char* arg_name = "argument")
  {
    char* ptr;
    ssize_t sz;
    PyBytes_AsStringAndSize(in.ptr(), &ptr, &sz);

    size_t size = static_cast<size_t>(sz);
    if (required_size && size != required_size)
        throw std::invalid_argument{"Invalid "s + arg_name + " value: expected " + std::to_string(required_size) + " bytes"};
    return ustring_view{reinterpret_cast<unsigned char*>(ptr), size};
  }

  void
  XEd25519_Init(py::module& mod)
  {
    using namespace pybind11::literals;
    auto submod = mod.def_submodule("xed25519");
    submod.def("sign", [](py::bytes curve25519_privkey, py::bytes msg) {
        auto a = uview(curve25519_privkey, 32, "curve25519_privkey");
        auto sig = xed25519::sign(a, uview(msg));
        return py::bytes(reinterpret_cast<const char*>(sig.data()), sig.size());
      },
      "curve25519_privkey"_a, "msg"_a,
      "Constructs an XEd25519 signature of `msg` using the given private key");

    submod.def("pubkey", [](py::bytes curve25519_pubkey) {
        auto A = uview(curve25519_pubkey, 32, "curve25519_pubkey");
        auto edpk = xed25519::pubkey(A);
        return py::bytes(reinterpret_cast<const char*>(edpk.data()), edpk.size());
      },
      "curve25519_pubkey"_a,
      "Returns the derived Ed25519 pubkey from a curve25519 pubkey");

    submod.def("verify", [](py::bytes signature, py::bytes curve25519_pubkey, py::bytes msg) {
            auto sig = uview(signature, 64, "signature");
            auto A = uview(curve25519_pubkey, 32, "curve25519_pubkey");
            return xed25519::verify(sig, A, uview(msg));
        },
        "signature"_a, "curve25519_pubkey"_a, "msg"_a,
        "Verifies the XEd25519 signature `signature` of `msg` allegedly signed by `curve25519_pubkey`.");
  }
}  // namespace onionreq
