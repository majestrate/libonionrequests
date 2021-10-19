#include "sogs_info.hpp"

namespace onionreq
{
  nlohmann::json
  SOGSInfo::ControlData() const
  {
    return nlohmann::json{{"host", hostname}, {"port", port}, {"target", "/loki/v3/lsrpc"}};
  }

  /// @brief make string representation for display purposes
  std::string
  SOGSInfo::ToString() const
  {
    return "[sogs: " + hostname + ":" + std::to_string(port) + "]";
  }

}  // namespace onionreq
