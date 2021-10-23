#include "common.hpp"

PYBIND11_MODULE(pyonionreq, m)
{
  onionreq::Common_Init(m);
  onionreq::Transport_Init(m);
  onionreq::OnionMaker_Init(m);
  onionreq::Consensus_Init(m);
  onionreq::Junk_Init(m);
}
