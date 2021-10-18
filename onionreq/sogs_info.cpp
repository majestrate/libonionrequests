#include "sogs_info.hpp"

namespace onionreq
{
  nlohmann::json
  SOGSInfo::ControlData() const
  {
    return nlohmann::json{{"host", hostname}, {"target", "/loki/v3/lsrpc"}};
  }

}  // namespace onionreq
