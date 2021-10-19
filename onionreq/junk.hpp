#pragma once

#include "channel_encryption.hpp"
#include "key_types.hpp"
#include "snode_info.hpp"
#include <optional>
#include <string>
#include <functional>

namespace onionreq
{
  /// @brief a bundle of junk we get from an onion request
  struct Junk
  {
    /// plaintext payload
    std::string payload;
    /// decrypt/encrypt reply
    std::function<std::string(std::string)> transformReply;
  };

  class JunkParser_Base
  {
   public:
    virtual ~JunkParser_Base() = default;

    /// @brief give a pile of stuff and decode it into some junk
    /// throws on decode error
    virtual Junk
    ParseJunk(std::string_view stuff) const = 0;
  };

  JunkParser_Base*
  JunkParser(x25519_keypair keypair);

  JunkParser_Base*
  JunkParser(SNodeInfo snode, EncryptType keytype);

  JunkParser_Base*
  JunkParser(x25519_pubkey other, EncryptType keytype);

}  // namespace onionreq
