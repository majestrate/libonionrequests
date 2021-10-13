#pragma once

#include <variant>
#include <vector>

#pragma once
#include "sogs_info.hpp"
#include "snode_info.hpp"

namespace onionreq
{
  /// @brief a remote resource we are talking to
  using RemoteResource_t = std::variant<SNodeInfo, SOGSInfo>;

  /// @brief the path that an onion request takes to some remote resource
  struct OnionPath
  {
    /// @brief the full hops we are using
    std::vector<SNodeInfo> hops;
    /// @brief the remote resource we are sending stuff to
    RemoteResource_t remote;

    /// @brief snode used as our first hop
    SNodeInfo& edge{hops[0]};
  };

  /// @brief the payload we send to the first hop
  struct OnionPayload
  {
    /// @brief ciphertext payload to send via transport
    std::string ciphertext;
    /// @brief the path this payload belongs to
    OnionPath path;
  };

}  // namespace onionreq
