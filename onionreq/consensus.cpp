#include "consensus.hpp"
#include "random.hpp"
#include <oxenc/bt_serialize.h>
#include <nlohmann/json.hpp>

#include <future>

namespace onionreq
{
  void
  PathSelection_Base::StoreNodeList(std::unordered_map<ed25519_pubkey, SNodeInfo> snodelist)
  {
    AccessList([&](auto& list) { list = snodelist; });
  }

  class PathSelection_Default : public PathSelection_Base
  {
    /// XXX: only access me while using AccessList
    std::unordered_set<SNodeInfo> _edges;

   public:
    int
    DefaultHopLength() const override
    {
      return 4;
    }

    std::optional<OnionPath>
    MaybeSelectHopsTo(RemoteResource_t remote) override
    {
      // maximum unique number of edges to use
      constexpr size_t MaxEdges = 4;
      OnionPath path;
      AccessList([&](const auto& list) {
        std::vector<typename std::decay<decltype(list)>::type::value_type> hops;
        std::sample(list.begin(), list.end(), std::back_inserter(hops), DefaultHopLength(), PRNG{});
        for (const auto& hop : hops)
          path.hops.push_back(hop.second);
        if (_edges.size() >= MaxEdges)
        {
          path.Edge() = pick_random_from<SNodeInfo>(_edges);
        }
        _edges.insert(path.Edge());
      });
      path.remote = remote;
      return path;
    }

    void
    OnionPathFailed(const OnionPath& path) override
    {
      (void)path;
    }

    void
    OnionPathSuccess(const OnionPath& path) override
    {
      (void)path;
    }
  };

  class OxenMQ_NodeFetcher_JSON : public NodeListFetcher_Base
  {
    oxenmq::OxenMQ& _mq;
    const std::unordered_set<std::string> _seeds;

   protected:
    std::optional<oxenmq::ConnectionID> _conn;

    std::optional<oxenmq::ConnectionID>
    ObtainConnection()
    {
      if (_conn)
        return _conn;
      std::promise<std::optional<oxenmq::ConnectionID>> _result;
      _mq.connect_remote(
          pick_random_from<oxenmq::address>(_seeds),
          [&_result](auto conn) { _result.set_value(std::optional<oxenmq::ConnectionID>{conn}); },
          [&_result](auto, auto) { _result.set_value(std::nullopt); });
      auto ftr = _result.get_future();
      _conn = ftr.get();
      return _conn;
    }

    virtual std::unordered_map<ed25519_pubkey, SNodeInfo>
    ParseNodeList(std::string_view data)
    {
      std::unordered_map<ed25519_pubkey, SNodeInfo> list;
      const auto val = nlohmann::json::parse(data);
      if (auto itr = val.find("service_node_states"); itr != val.end())
      {
        for (const auto& entry : *itr)
        {
          SNodeInfo info{entry};
          list[info.identity] = info;
        }
      }
      return list;
    }

    virtual std::string_view
    RPCMethod() const
    {
      return "rpc.get_service_nodes";
    }

    virtual std::string
    RPCBody() const
    {
      const nlohmann::json req{
          {"fields",
           {{"pubkey_ed25519", true},
            {"pubkey_x25519", true},
            {"public_ip", true},
            {"storage_port", true},
            {"storage_lmq_port", true}}}};
      return req.dump();
    }

   public:
    explicit OxenMQ_NodeFetcher_JSON(oxenmq::OxenMQ& mq, std::unordered_set<std::string> seeds)
        : _mq{mq}, _seeds{std::move(seeds)}
    {}

    void
    FetchAll(std::function<void(std::unordered_map<ed25519_pubkey, SNodeInfo>)> callback) override
    {
      if (auto maybe = ObtainConnection())
      {
        _mq.request(
            *maybe,
            RPCMethod(),
            [callback, this](bool success, auto msgs) {
              std::unordered_map<ed25519_pubkey, SNodeInfo> nodes;
              if (success and msgs.size() > 1)
              {
                try
                {
                  nodes = ParseNodeList(msgs[1]);
                }
                catch (...)
                {
                  // discard exceptions in parsing
                }
              }
              callback(std::move(nodes));
            },
            RPCBody());
      }
      else
      {
        callback({});
      }
    }

    void
    Fetch(ed25519_pubkey ident, std::function<void(std::optional<SNodeInfo>)> callback) override
    {
      callback(std::nullopt);
    }
  };

  void
  Consensus_Base::SeedNodes(std::unordered_set<std::string> seeds)
  {
    _seeds = std::move(seeds);
  }

  class Consensus_Direct : public Consensus_Base
  {
    oxenmq::OxenMQ& _mq;

   public:
    explicit Consensus_Direct(oxenmq::OxenMQ& mq) : _mq{mq}
    {}

    NodeListFetcher_Base*
    CreateNodeFetcher() const override
    {
      return new OxenMQ_NodeFetcher_JSON{_mq, _seeds};
    }

    PathSelection_Base*
    CreatePathSelector() const override
    {
      return new PathSelection_Default{};
    }
  };

  Consensus_Base*
  Consensus(direct_oxenmq, oxenmq::OxenMQ& omq)
  {
    return new Consensus_Direct{omq};
  }

  Consensus_Base*
  Consensus(lokinet_oxenmq, oxenmq::OxenMQ& omq)
  {
    return nullptr;
  }
}  // namespace onionreq
