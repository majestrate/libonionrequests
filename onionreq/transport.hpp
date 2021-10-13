#pragma once

#include "snode_info.hpp"
#include "onion.hpp"
#include "lokinet.hpp"

#include <oxenmq/oxenmq.h>
#include <functional>

namespace onionreq
{
  /// @brief basae type for transporting an onion request to the first node
  class Transport_Base
  {
   public:
    virtual ~Transport_Base() = default;

    virtual void
    SendToEdge(
        const SNodeInfo& edge,
        OnionPayload payload,
        std::function<void(std::error_code)> completionHandler) = 0;
  };

  Transport_Base*
  Transport(const std::shared_ptr<oxenmq::OxenMQ>&);

  Transport_Base*
  Transport(const std::shared_ptr<lokinet::Context>&);

}  // namespace onionreq
