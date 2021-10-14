#pragma once

#include <array>
#include <string_view>

namespace std
{
  template <>
  struct hash<std::array<char, 32>>
  {
    size_t
    operator()(const std::array<char, 32>& k) const
    {
      return std::hash<std::string_view>{}(std::string_view{k.data(), k.size()});
    }
  };

}  // namespace std
