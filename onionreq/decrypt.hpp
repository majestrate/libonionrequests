#pragma once

#include "key_types.hpp"
#include <optional>
#include <string>
#include <functional>

namespace onionreq
{
  /// @brief a bundle of junk we get from decrypting an onion request
  struct Junk
  {
    /// plaintext payload
    std::string payload;
    /// encrypt paintext data for reply
    std::function<std::string(std::string)> encryptReply;
  };

  class JunkDecrypter
  {
    x25519_keypair _keys;

   public:
    explicit JunkDecrypter(x25519_keypair keys);

    /// @brief give this decoder a pile of stuff and decode it into some junk
    /// throws on decode error
    Junk
    DecryptJunk(std::string_view junk) const;
  };

}  // namespace onionreq
