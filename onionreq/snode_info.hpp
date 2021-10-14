#pragma once

#include <cstdint>
#include <string>

#include "key_types.hpp"

#include <oxenmq/bt_value.h>
#include <nlohmann/json.hpp>

namespace onionreq
{
  /// @brief info on a service node
  struct SNodeInfo
  {
    /// identity key for .snode
    std::array<char, 32> identity;
    /// x25519 key for onion requests
    std::array<char, 32> onion;
    /// public internet ip
    std::string publicIP;
    /// storage server port
    uint16_t storagePort;

    SNodeInfo() = default;

    /// @brief construct from json object
    explicit SNodeInfo(nlohmann::json object);
    /// @brief construct from bt dict
    explicit SNodeInfo(oxenmq::bt_dict dict);

    /// @brief compute .snode address as string
    std::string
    SNodeAddr() const;
  };

}  // namespace onionreq
