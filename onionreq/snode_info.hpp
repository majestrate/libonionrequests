#pragma once

#include <cstdint>
#include <string>

#include "key_types.hpp"

#include <oxenc/bt_value.h>
#include <nlohmann/json.hpp>

namespace onionreq
{
  /// @brief info on a service node
  struct SNodeInfo
  {
    /// identity key for .snode
    ed25519_pubkey identity;
    /// x25519 key for onion requests
    x25519_pubkey onion;
    /// public internet ip
    std::string publicIP;
    /// storage server zmq port
    uint16_t zmqPort;
    /// storage server https port
    uint16_t httpsPort;

    SNodeInfo() = default;

    /// @brief construct from json object
    explicit SNodeInfo(const nlohmann::json& object);
    /// @brief construct from bt dict
    explicit SNodeInfo(const oxenc::bt_dict& dict);

    nlohmann::json
    ControlData() const;

    /// @brief compute the omq connection string via lokinet
    std::string
    SNodeAddr() const;

    /// @brief compute the direct omq connection string
    std::string
    DirectAddr() const;

    std::string
    HttpsDirect() const;

    std::string
    HttpsSNode() const;

    bool
    operator==(const SNodeInfo& other) const
    {
      return identity == other.identity;
    }
  };

}  // namespace onionreq

namespace std
{
  template <>
  struct hash<onionreq::SNodeInfo>
  {
    size_t
    operator()(const onionreq::SNodeInfo& info) const
    {
      return std::hash<onionreq::ed25519_pubkey>{}(info.identity);
    }
  };
}  // namespace std
