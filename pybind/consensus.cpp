#include "common.hpp"
#include <onionreq/consensus.hpp>
#include <future>

namespace onionreq
{
  oxenmq::OxenMQ _mq;

  void
  Consensus_Init(py::module& mod)
  {
    auto submod = mod.def_submodule("consensus");
    py::class_<Consensus_Base, std::shared_ptr<Consensus_Base>>(submod, "ConsenusBase")
        .def(
            "make_path_selector",
            [](const Consensus_Base& self) {
              return std::shared_ptr<PathSelection_Base>{self.CreatePathSelector()};
            })
        .def("make_node_fetcher", [](const Consensus_Base& self) {
          return std::shared_ptr<NodeListFetcher_Base>{self.CreateNodeFetcher()};
        });

    py::class_<NodeListFetcher_Base, std::shared_ptr<NodeListFetcher_Base>>(
        submod, "NodeFetcherBase");

    py::class_<PathSelection_Base, std::shared_ptr<PathSelection_Base>>(submod, "PathSelectorBase")
        .def(
            "set_node_list",
            [](const std::shared_ptr<PathSelection_Base>& self,
               std::unordered_map<PublicIdentityKey_t, SNodeInfo> nodelist) {
              self->StoreNodeList(std::move(nodelist));
            });

    submod.def("direct", []() {
      return std::shared_ptr<Consensus_Base>{Consensus(direct_oxenmq{}, _mq)};
    });
    submod.def("lokinet", []() {
      return std::shared_ptr<Consensus_Base>{Consensus(lokinet_oxenmq{}, _mq)};
    });
  }
}  // namespace onionreq
