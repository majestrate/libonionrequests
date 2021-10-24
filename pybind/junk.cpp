#include "common.hpp"
#include <onionreq/junk.hpp>
#include <sodium/crypto_box.h>

namespace onionreq
{
  class PyJunkParser
  {
    std::unique_ptr<JunkParser_Base> _impl;

   public:
    explicit PyJunkParser(py::bytes pubkey, py::bytes privkey) : _impl{nullptr}
    {
      x25519_keypair keys{};

      auto set_key = [](auto& out, const py::bytes& in) {
        char* ptr{};
        ssize_t _sz{};

        PyBytes_AsStringAndSize(in.ptr(), &ptr, &_sz);

        if (_sz != out.size())
          throw std::invalid_argument{"invalid pubkey size"};

        std::copy_n(ptr, _sz, out.begin());
      };

      set_key(keys.first, pubkey);
      set_key(keys.second, privkey);

      _impl.reset(JunkParser(std::move(keys)));
    }

    Junk
    Parse(std::string_view stuff) const
    {
      return _impl->ParseJunk(std::move(stuff));
    }
  };

  void
  Junk_Init(py::module& mod)
  {
    auto submod = mod.def_submodule("junk");
    py::class_<Junk>(submod, "Junk")
        .def(
            "transformReply",
            [](const Junk& junk, std::string replyStr) -> std::string {
              return junk.transformReply(std::move(replyStr));
            })
        .def_readonly("payload", &Junk::payload);

    py::class_<PyJunkParser>(submod, "Parser")
        .def(py::init<py::bytes, py::bytes>())
        .def("parse_junk", [](const PyJunkParser& self, std::string_view stuff) -> Junk {
          return self.Parse(std::move(stuff));
        });
  }
}  // namespace onionreq
