#include "common.hpp"
#include <onionreq/consensus.hpp>
#include <future>

namespace onionreq
{
  void
  Consensus_Init(py::module& mod)
  {
    auto submod = mod.def_submodule("consensus");
    py::class_<Consensus_Base, std::shared_ptr<Consensus_Base>>(submod, "ConsenusBase")
        .def(
            "make_path_selector",
            [](const std::shared_ptr<Consensus_Base>& self) {
              return std::shared_ptr<PathSelection_Base>{self->CreatePathSelector()};
            })
        .def("make_node_fetcher", [](const std::shared_ptr<Consensus_Base>& self) {
          return std::shared_ptr<NodeListFetcher_Base>{self->CreateNodeFetcher()};
        });

    py::class_<NodeListFetcher_Base, std::shared_ptr<NodeListFetcher_Base>>(
        submod, "NodeFetcherBase")
        .def(
            "fetch_one",
            [](const std::shared_ptr<NodeListFetcher_Base>& self,
               const PublicIdentityKey_t& ident) {
              std::promise<std::optional<SNodeInfo>> result;
              self->Fetch(ident, [&result](auto maybe) { result.set_value(maybe); });
              auto ftr = result.get_future();
              return ftr.get();
            })
        .def("fetch_all", [](const std::shared_ptr<NodeListFetcher_Base>& self) {
          std::promise<std::unordered_map<PublicIdentityKey_t, SNodeInfo>> result;
          self->FetchAll([&result](auto map) { result.set_value(std::move(map)); });
          auto ftr = result.get_future();
          return ftr.get();
        });

    py::class_<PathSelection_Base, std::shared_ptr<PathSelection_Base>>(submod, "PathSelectorBase")
        .def(
            "make_path",
            [](const std::shared_ptr<PathSelection_Base>& self, RemoteResource_t remote) {
              return self->MaybeSelectHopsTo(std::move(remote));
            })
        .def(
            "path_result",
            [](const std::shared_ptr<PathSelection_Base>& self,
               const OnionPath& path,
               std::error_code result) { self->OnionPathResult(path, result); });

    submod.def(
        "direct", []() { return std::shared_ptr<Consensus_Base>{Consensus(direct_oxenmq{})}; });
    submod.def(
        "lokinet", []() { return std::shared_ptr<Consensus_Base>{Consensus(lokinet_oxenmq{})}; });
  }
}  // namespace onionreq
