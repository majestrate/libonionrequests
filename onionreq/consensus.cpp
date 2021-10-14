#include "consensus.hpp"

namespace onionreq
{
  void
  PathSelection_Base::StoreNodeList(std::unordered_map<PublicIdentityKey_t, SNodeInfo> snodelist)
  {
    _snodelist = std::move(snodelist);
  }

  class PathSelection_Default : public PathSelection_Base
  {
   public:
    std::optional<OnionPath>
    MaybeSelectHopsTo(RemoteResource_t remote) override
    {
      // TODO: implement me
      return std::nullopt;
    }

    void
    OnionPathResult(const OnionPath& path, std::error_code result) override
    {}
  };

  class OxenMQ_NodeFetcher : public NodeListFetcher_Base
  {
    oxenmq::OxenMQ& _mq;
    const std::unordered_set<std::string> _seeds;

   public:
    explicit OxenMQ_NodeFetcher(oxenmq::OxenMQ& mq, std::unordered_set<std::string> seeds)
        : _mq{mq}, _seeds{std::move(seeds)}
    {}

    void
    FetchAll(
        std::function<void(std::unordered_map<PublicIdentityKey_t, SNodeInfo>)> callback) override
    {}

    void
    Fetch(
        PublicIdentityKey_t ident, std::function<void(std::optional<SNodeInfo>)> callback) override
    {}
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
      return new OxenMQ_NodeFetcher{_mq, _seeds};
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
