#include "transport.hpp"
#include <future>
#include <mutex>

#include <spdlog/spdlog.h>

namespace onionreq
{
  class Transport_OMQ : public Transport_Base
  {
    oxenmq::OxenMQ& _mq;
    std::mutex _access;
    std::unordered_map<ed25519_pubkey, oxenmq::ConnectionID> _edges;

    std::optional<oxenmq::ConnectionID>
    ObtainConnectionTo(const SNodeInfo& info)
    {
      // check cached connections
      {
        std::unique_lock _lock{_access};
        if (auto itr = _edges.find(info.identity); itr != _edges.end())
          return itr->second;
      }
      std::promise<std::optional<oxenmq::ConnectionID>> result;
      _mq.connect_remote(
          oxenmq::address{info.DirectAddr()},
          [&result](auto conn) { result.set_value(std::optional<oxenmq::ConnectionID>{conn}); },
          [&result](auto, auto) { result.set_value(std::nullopt); });
      auto ftr = result.get_future();
      if (auto maybe = ftr.get())
      {
        // put cached result if found
        std::unique_lock _lock{_access};
        auto itr = _edges.emplace(info.identity, *maybe);
        return itr.first->second;
      }
      return std::nullopt;
    }

   public:
    explicit Transport_OMQ(oxenmq::OxenMQ& mq) : _mq{mq}
    {}

    void
    SendPayload(
        OnionPayload payload,
        std::function<void(std::optional<std::string>)> responseHandler) override
    {
      if (auto maybe = ObtainConnectionTo(payload.path.Edge()))
      {
        _mq.request(
            *maybe,
            "sn.onion_request",
            [responseHandler](bool success, auto msgs) {
              if (success and msgs.size() > 1)
              {
                responseHandler(msgs[1]);
              }
              else
              {
                spdlog::error("success={} msgs={}", success, msgs.size());
                if (not msgs.empty())
                  spdlog::error("error={}", msgs[0]);
                responseHandler(std::nullopt);
              }
            },
            payload.ciphertext);
      }
      else
      {
        spdlog::error("Cannot get connection");
        responseHandler(std::nullopt);
      }
    }
  };

  Transport_Base*
  Transport(oxenmq::OxenMQ& mq)
  {
    return new Transport_OMQ{mq};
  }

}  // namespace onionreq
