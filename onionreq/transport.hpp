#pragma once

#include "context_types.hpp"
#include "snode_info.hpp"
#include "onion_payload.hpp"

#include <functional> 

namespace onionreq
{
  /// @brief basae type for transporting an onion request to the first node 
  class Transport_Base
  {
  public:
    virtual ~Transport_Base() = default;

    virtual void
    SendToEdge(const SNodeInfo & edge, OnionPayload payload, std::function<void(std::error_code)> completionHandler) = 0;
    
  };


  Transport_Base *
  Transport(const std::shared_ptr<OxenMQ> &);
  
  Transport_Base *
  Transport(const std::shared_ptr<Lokinet> &);
  
} 
  
