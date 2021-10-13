#pragma once 

namespace onionreq
{
  
} 


void onion_request(std::string ip, uint16_t port, std::vector<std::pair<ed25519_pubkey, x25519_pubkey>> keys,
        bool mainnet, std::optional<EncryptType> enc_type, std::string_view payload, std::string_view control);
