#include "snode_info.hpp"
#include <oxenc/hex.h>
#include <oxenc/base32z.h>

namespace onionreq
{
  namespace
  {
    template <typename OutType>
    OutType&
    LoadFromHex(const std::string& str, OutType& out)
    {
      if (oxenc::from_hex_size(str.size()) > out.size())
        throw std::range_error{
            "input too big: " + std::to_string(str.size()) + " > " + std::to_string(out.size())};
      oxenc::from_hex(str.begin(), str.end(), out.begin());
      return out;
    }
  }  // namespace

  SNodeInfo::SNodeInfo(const nlohmann::json& obj)
  {
    LoadFromHex(obj.at("pubkey_ed25519").get<std::string>(), identity);
    LoadFromHex(obj.at("pubkey_x25519").get<std::string>(), onion);
    publicIP = obj.at("public_ip").get<std::string>();
    zmqPort = obj.at("storage_lmq_port").get<uint16_t>();
    httpsPort = obj.at("storage_port").get<uint16_t>();
  }

  std::string
  SNodeInfo::SNodeAddr() const
  {
    return "curve://" + oxenc::to_base32z(identity.begin(), identity.end())
        + ".snode:" + std::to_string(zmqPort) + "/" + oxenc::to_hex(onion.begin(), onion.end());
  }

  std::string
  SNodeInfo::DirectAddr() const
  {
    return "curve://" + publicIP + ":" + std::to_string(zmqPort) + "/"
        + oxenc::to_hex(onion.begin(), onion.end());
  }

  std::string
  SNodeInfo::HttpsSNode() const
  {
    return "https://" + oxenc::to_base32z(identity.begin(), identity.end()) + ":"
        + std::to_string(httpsPort) + "/onion_req/v2";
  }

  std::string
  SNodeInfo::HttpsDirect() const
  {
    return "https://" + publicIP + ":" + std::to_string(httpsPort) + "/onion_req/v2";
  }

  nlohmann::json
  SNodeInfo::ControlData() const
  {
    return nlohmann::json{{"headers", {}}};
  }

}  // namespace onionreq
