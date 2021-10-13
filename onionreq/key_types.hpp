#pragma once

#include <array>
#include <string_view>

namespace onionreq
{

  /// @brief public identity key for the .snode address
  using PublicIdentityKey_t = std::array<char, 32>;

}

namespace std
{
  template <> struct hash<onionreq::PublicIdentityKey_t>
  {
    size_t
    operator()(const onionreq::PublicIdentityKey_t &k) const
    {
      return std::hash<std::string_view>{}(std::string_view{k.data(), k.size()});
    }
  };
} 
