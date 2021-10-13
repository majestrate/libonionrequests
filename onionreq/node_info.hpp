#pragma once

namespace onionreq
{
  /// @brief info on a service node
  struct SNodeInfo
  {
    PublicIdentityKey_t identity;
    PublicOnionKey_t onion;
    std::string publicIP;
    uint16_t storagePort;
  };

} 
