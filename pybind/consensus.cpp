#include "common.hpp"
#include <onionreq/consensus.hpp>
#include <future>

namespace onionreq
{
  class PyDirectConsensus : public Holder<Consensus_Base>
  {
    oxenmq::OxenMQ _mq;

   public:
    explicit PyDirectConsensus(std::vector<std::string> seeds) : Holder<Consensus_Base>{nullptr}
    {
      if (seeds.empty())
        throw std::invalid_argument{"cannot use empty seed node list"};
      std::unordered_set<std::string> _seeds{seeds.begin(), seeds.end()};
      impl.reset(Consensus(direct_oxenmq{}, _mq));
      impl->SeedNodes(std::move(_seeds));
      _mq.start();
    }
  };

  void
  Consensus_Init(py::module& mod)
  {
    auto submod = mod.def_submodule("consensus");

    py::class_<Holder<NodeListFetcher_Base>>(submod, "NodeFetcherBase")
        .def(
            "fetch_all",
            [](const Holder<NodeListFetcher_Base>& self,
               std::function<void(std::unordered_map<ed25519_pubkey, SNodeInfo>)> callback) {
              py::gil_scoped_release rel{};
              self.impl->FetchAll([callback](auto nodes) {
                py::gil_scoped_acquire gil{};
                callback(std::move(nodes));
              });
            });

    py::class_<Holder<PathSelection_Base>>(submod, "PathSelectorBase")
        .def(
            "set_node_list",
            [](const Holder<PathSelection_Base>& self,
               std::unordered_map<ed25519_pubkey, SNodeInfo> nodelist) {
              self.impl->StoreNodeList(std::move(nodelist));
            });

    py::class_<PyDirectConsensus>(submod, "Direct")
        .def(py::init<std::vector<std::string>>())
        .def(
            "make_path_selector",
            [](const PyDirectConsensus& self) {
              return Holder<PathSelection_Base>{self.impl->CreatePathSelector()};
            })
        .def("make_node_fetcher", [](const PyDirectConsensus& self) {
          return Holder<NodeListFetcher_Base>{self.impl->CreateNodeFetcher()};
        });
  }
}  // namespace onionreq
