#include "decrypt.hpp"
#include "channel_encryption.hpp"
#include <string_view>
#include <endian.h>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace onionreq
{
  namespace
  {
    uint32_t
    maybe_decode_size(std::string_view data)
    {
      if (data.size() < sizeof(uint32_t))
        throw std::invalid_argument{"payload for junk decrypter too small"};
      return htole32(*reinterpret_cast<const uint32_t*>(data.data()));
    }

  }  // namespace

  JunkDecrypter::JunkDecrypter(x25519_keypair keys) : _keys{keys}
  {}

  Junk
  JunkDecrypter::DecryptJunk(std::string_view data) const
  {
    auto sz = maybe_decode_size(data);
    EncryptType keytype{EncryptType::aes_gcm};
    auto ciphertext = data.substr(sizeof(sz), sizeof(sz) + sz);
    auto metadata = nlohmann::json::parse(data.substr(sizeof(sz) + sz));

    x25519_pubkey remote_pk{};

    if (auto itr = metadata.find("ephemeral_key"); itr != metadata.end())
      remote_pk = parse_x25519_pubkey(itr->get<std::string>());
    else
      throw std::invalid_argument{"metadata does not have 'ephemeral_key' entry"};

    Junk junk{};

    ChannelEncryption d{_keys.second, _keys.first};

    junk.payload = d.decrypt(keytype, ciphertext, remote_pk);
    junk.encryptReply =
        [e = ChannelEncryption{_keys.second, _keys.first, false}, keytype, remote_pk](
            std::string_view plaintext) { return e.encrypt(keytype, plaintext, remote_pk); };
    return junk;
  }

}  // namespace onionreq
