#include "onion_maker.hpp"

namespace onionreq
{
  class OnionMaker_AESGCM : public OnionMaker_Base
  {
    //
   public:
    OnionMaker_AESGCM()
    {}

    OnionPayload
    MakeOnionFromHops(std::string plaintext, const OnionPath& path) const override
    {
      OnionPayload onion{};
      onion.path = path;

      return onion;
    }
  };

  OnionMaker_Base* OnionMaker(all_aesgcm_hops)
  {
    return new OnionMaker_AESGCM{};
  }

  OnionMaker_Base* OnionMaker(all_xchacha20_hops)
  {
    // TODO: implement me
    return nullptr;
  }
}  // namespace onionreq
