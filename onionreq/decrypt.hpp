#pragma once

#include "key_types.hpp"
#include <optional>
#include <string>
#include <functional>

namespace onionreq
{
  struct Junk
  {
    std::string payload;
    std::function<std::string(std::string)> encryptReply;
  };

  class JunkDecrypter
  {
    x25519_keypair _keys;

   public:
    explicit JunkDecrypter(x25519_keypair keys);

    /// @brief give this decoder a pile of stuff and decode it into plaintext
    Junk
    DecryptJunk(std::string_view junk) const;
  };

}  // namespace onionreq
