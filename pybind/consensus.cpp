#pragma once
#include "common.hpp"
#include <onionreq/consensus.hpp>

namespace onionreq
{
  void Consensus_Init(py::module &mod)
  {
    auto submod = mod.def_submodule("consensus");
    py::class_<Consenus_Base, std::unique_ptr<Consensus_Base>>(submod, "ConsenusBase");

    py::class_<NodeListFetcher_Base, std::unique_ptr<NodeListFetcher_Base>>(submod, "NodeFetcherBase")
      .def(py::init<const std::unique_ptr<Consensus_Base>&>{[](const auto & base) {
        return std::unique_ptr<NodeListFetcher_Base>{base->CreateNodeFetcher()};
      }})
      .def("fetch_one", [](const std::unique_ptr<NodeListFetcher_Base> & self, const PublicIdentityKey_t & ident)
      {
        std::promise<std::optional<SNodeInfo>> result;
        self->Fetch(ident, [&result](auto maybe) { result.set_value(maybe); });
        auto ftr = result.get_future();
        return ftr.get();
      })
      .def"fetch_all",  [](const std::unique_ptr<NodeListFetcher_Base> & self)
      {
        std::promise<std::unordered_map<PublicIdentityKey_t, SNodeInfo>> result;
        self->FetchAll([&result](auto map) { result.set_value(std::move(map)); });
        auto ftr = result.get_future();
        return ftr.get();
      });

    submod.def("direct", []() { return std::uniuqe_ptr<Consensus_Base>{Consensus(direct_oxenmq{})}; });
    submod.def("lokinet", []() { return std::uniuqe_ptr<Consensus_Base>{Consensus(lokinet_oxenmq{})}; });
  }
} // namespace onionreq
