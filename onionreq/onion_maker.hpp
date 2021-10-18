#pragma once
#include "onion.hpp"

namespace onionreq
{
  class OnionMaker_Base
  {
   public:
    virtual ~OnionMaker_Base() = default;

    /// @brief given an onion path, make an onion request, encrypt it and all that stuff
    virtual OnionPayload
    MakeOnion(std::string_view plaintext, const OnionPath& path) const = 0;
  };

  struct all_xchacha20_hops
  {};
  struct all_aesgcm_hops
  {};

  /// @brief make an onion maker that uses chacha20 for all hops
  OnionMaker_Base* OnionMaker(all_xchacha20_hops);

  /// @brief cosntruct an onion maker that uses aesgcm for all hops
  OnionMaker_Base* OnionMaker(all_aesgcm_hops);

}  // namespace onionreq
