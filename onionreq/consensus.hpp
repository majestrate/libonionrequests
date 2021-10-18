#pragma once

#include "key_types.hpp"
#include "snode_info.hpp"
#include "onion.hpp"

#include <functional>
#include <memory>
#include <system_error>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

#include <oxenmq/oxenmq.h>

namespace onionreq
{
  /// @brief fetcher of snode list
  class NodeListFetcher_Base
  {
   public:
    virtual ~NodeListFetcher_Base() = default;

    /// @brief fetch the entire service node list
    virtual void
    FetchAll(std::function<void(std::unordered_map<ed25519_pubkey, SNodeInfo>)> callback) = 0;

    /// @brief fetch just one service node inf by its public identity key
    virtual void
    Fetch(ed25519_pubkey ident, std::function<void(std::optional<SNodeInfo>)> callback) = 0;
  };

  /// @brief a path selection algorithm that determines how we build paths and gives a way to
  /// propagate feedback on errors and success
  class PathSelection_Base
  {
    std::unordered_map<ed25519_pubkey, SNodeInfo> _snodelist;
    std::mutex _access;

   protected:
    template <typename Visit>
    void
    AccessList(Visit&& visit)
    {
      std::unique_lock _lock{_access};
      visit(_snodelist);
    }

   public:
    virtual ~PathSelection_Base() = default;

    /// @brief give this path selector the nodes to choose from when doing path selection
    void
    StoreNodeList(std::unordered_map<ed25519_pubkey, SNodeInfo> snodelist);

    /// @brief maybe select some hops to go to a remote resource
    virtual std::optional<OnionPath>
    MaybeSelectHopsTo(RemoteResource_t remote) = 0;

    /// @brief get the number of hops used in a path that is used by default
    virtual int
    DefaultHopLength() const = 0;

    /// @brief record the result of an onion request along a path failing
    virtual void
    OnionPathFailed(const OnionPath& path) = 0;

    /// @brief record the result of an onion request along a path succeeding
    virtual void
    OnionPathSuccess(const OnionPath& path) = 0;
  };

  /// @brief use oxenmq directly
  struct direct_oxenmq
  {};

  /// @brief use oxenmq over lokinet
  struct lokinet_oxenmq
  {};

  /// @brief consensus type that creates all concrete implementations for
  /// consensus
  class Consensus_Base
  {
   protected:
    std::unordered_set<std::string> _seeds;

   public:
    virtual ~Consensus_Base() = default;

    /// @brief override seed nodes
    void
    SeedNodes(std::unordered_set<std::string> seeds);

    /// @brief construct a new nodefetcher
    virtual NodeListFetcher_Base*
    CreateNodeFetcher() const = 0;

    /// @brief construct a new path selection algorithm
    virtual PathSelection_Base*
    CreatePathSelector() const = 0;

    /// TODO: more consensus related stuff here
  };

  /// @brief make a lokinet based consensus fetcher
  Consensus_Base*
  Consensus(lokinet_oxenmq, oxenmq::OxenMQ&);

  /// @brief make a oxenmq based consensus fetcher
  Consensus_Base*
  Consensus(direct_oxenmq, oxenmq::OxenMQ&);

}  // namespace onionreq
