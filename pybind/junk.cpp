#include "common.hpp"
#include <onionreq/junk.hpp>
#include <sodium/crypto_box.h>

namespace onionreq
{
  template <typename T>
  static void
  set_key(T& out, const py::bytes& in)
  {
    char* ptr;
    ssize_t sz;
    PyBytes_AsStringAndSize(in.ptr(), &ptr, &sz);

    if (sz != out.size())
      throw std::invalid_argument{"invalid pubkey size"};

    std::copy_n(ptr, sz, out.begin());
  }

  void
  Junk_Init(py::module& mod)
  {
    using namespace pybind11::literals;
    auto submod = mod.def_submodule("junk");
    py::class_<Junk>(
        submod,
        "Junk",
        "Class holding a parsed reply that is able to encrypt a reply to the message sender")
        .def(
            "transformReply",
            [](const Junk& junk, py::bytes replyStr) -> py::bytes {
              return junk.transformReply(replyStr);
            },
            "reply"_a,
            "Takes a bytes value to send in reply and encrypts it for the original requestor.")
        .def_property_readonly(
            "payload",
            [](const Junk& junk) -> py::bytes { return junk.payload; },
            "The plaintext, decrypted data from the onion request");

    py::class_<JunkParser_Base>(submod, "Parser")
        .def(
            py::init([](py::bytes pubkey, py::bytes privkey) {
              x25519_keypair keys{};
              set_key(keys.first, pubkey);
              set_key(keys.second, privkey);
              return JunkParser(std::move(keys));
            }),
            py::kw_only(),
            "pubkey"_a,
            "privkey"_a,
            "Creates a parser that can parse and decrypt an onion request payload")
        .def(
            "parse_junk",
            [](const JunkParser_Base& self, py::bytes stuff_bytes) -> Junk {
              Junk j;
              {
                std::string stuff = stuff_bytes;
                py::gil_scoped_release gil_rel;
                j = self.ParseJunk(stuff);
              }
              return j;
            },
            "stuff"_a,
            "Parses and decrypts `stuff` from an onion request.");
  }
}  // namespace onionreq
