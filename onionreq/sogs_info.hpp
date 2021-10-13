#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace onionreq
{
  /// @brief what protocol the SOGS endpoint speaks
  enum class SOGSProtocol
  {
    http,
    https
  };

  /// @brief information about a SOGS endpoint
  struct SOGSInfo
  {
    SOGSProtocol protocol;
    std::string pubkey;
    std::string hostname;
    uint16_t port;

    explicit SOGSInfo(std::string_view url);
  };
}  // namespace onionreq
