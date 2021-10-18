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
    SendPayload(
        OnionPayload payload, std::function<void(std::optional<std::string>)> responseHandler) = 0;
  };

  Transport_Base*
  Transport(oxenmq::OxenMQ&);

}  // namespace onionreq
