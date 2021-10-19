#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <nlohmann/json.hpp>
#include "key_types.hpp"

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
    x25519_pubkey onion;
    std::string hostname;
    uint16_t port;

    /// @brief make string representation for display purposes
    std::string
    ToString() const;

    nlohmann::json
    ControlData() const;
  };
}  // namespace onionreq
