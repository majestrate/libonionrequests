#pragma once

#include <functional>
#include <variant>
#include <vector>
#include <optional>

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
    const SNodeInfo&
    Edge() const
    {
      return hops[0];
    }
    SNodeInfo&
    Edge()
    {
      return hops[0];
    }
  };

  /// @brief the payload we send to the first hop
  struct OnionPayload
  {
    /// @brief ciphertext payload to send via transport
    std::string ciphertext;
    /// @brief the path this payload belongs to
    OnionPath path;

    /// @brief take in a function that handles plaintext and wrap it so that it is called when we
    /// decrypt ciphertext
    /// @return a function you give ciphertext that will feed the plaintext decrypted value into the
    /// plaintext handler
    std::function<std::optional<std::string>(std::string)> maybeDecryptResponse;

    template <typename HandlerFunc>
    auto
    MakeDecrypter(HandlerFunc handler) const
    {
      return [handler = std::move(handler), this](std::string ct) {
        handler(maybeDecryptResponse(std::move(ct)));
      };
    }
  };
}  // namespace onionreq
