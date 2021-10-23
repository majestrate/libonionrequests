#include "common.hpp"
#include <onionreq/junk.hpp>
#include <sodium/crypto_box.h>

namespace onionreq
{
  class PyJunkParser
  {
    std::unique_ptr<JunkParser_Base> _impl;

   public:
    explicit PyJunkParser(py::bytes seed) : _impl{nullptr}
    {
      x25519_keypair keys{};

      char* _seed{};
      ssize_t _sz{};

      PyBytes_AsStringAndSize(seed.ptr(), &_seed, &_sz);

      if (_sz != crypto_box_SEEDBYTES)
        throw std::invalid_argument{"invalid seed size"};

      crypto_box_seed_keypair(
          keys.first.data(), keys.second.data(), reinterpret_cast<const uint8_t*>(_seed));

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
        .def(py::init<py::bytes>())
        .def("parse_junk", [](const PyJunkParser& self, std::string_view stuff) -> Junk {
          return self.Parse(std::move(stuff));
        });
  }
}  // namespace onionreq
