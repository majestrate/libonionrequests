#include <onionreq/onionreq.hpp>


int main(int argc, char * argv[])
{

  oxenmq::OxenMQ mq{};
  
  std::unique_ptr<onionreq::Consensus_Base> consensus
    {onionreq::Consensus(onionreq::direct_oxenmq{}, mq)};


  mq.start();

  
  return 0;
}
