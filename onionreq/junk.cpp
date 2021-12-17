#include "junk.hpp"
#include "channel_encryption.hpp"
#include <string_view>
#include <stdexcept>

#include <nlohmann/json.hpp>
#include <oxenc/endian.h>

namespace onionreq
{
  namespace
  {
    uint32_t
    decode_size(std::string_view data)
    {
      if (data.size() < sizeof(uint32_t))
        throw std::invalid_argument{"payload for junk decrypter too small"};
      return oxenc::load_little_to_host<uint32_t>(data.data());
    }

  }  // namespace

  class JunkDecrypter : public JunkParser_Base
  {
    x25519_keypair _keys;

   public:
    explicit JunkDecrypter(x25519_keypair keys) : _keys{keys}
    {}

    Junk
    ParseJunk(std::string_view data) const override
    {
      auto sz = decode_size(data);
      data.remove_prefix(sizeof(sz));
      if (data.size() < sz)
        throw std::invalid_argument{"encrypted data segment too small"};
      auto ciphertext = data.substr(0, sz);
      data.remove_prefix(sz);
      auto metadata = nlohmann::json::parse(data);

      x25519_pubkey remote_pk{};

      // defaults to aes-gcm if not provided
      EncryptType keytype{EncryptType::aes_gcm};

      if (auto encit = metadata.find("enc_type"); encit != metadata.end())
        keytype = parse_enc_type(encit->get<std::string_view>());

      if (auto itr = metadata.find("ephemeral_key"); itr != metadata.end())
        remote_pk = parse_x25519_pubkey(itr->get<std::string>());
      else
        throw std::invalid_argument{"metadata does not have 'ephemeral_key' entry"};

      Junk junk{};

      ChannelEncryption enc{_keys.second, _keys.first};

      junk.payload = enc.decrypt(keytype, ciphertext, remote_pk);
      junk.transformReply = [enc, keytype, remote_pk](std::string_view plaintext) {
        return enc.encrypt(keytype, plaintext, remote_pk);
      };
      return junk;
    }
  };

  JunkParser_Base*
  JunkParser(SNodeInfo info, EncryptType keytype)
  {
    // TODO: implement me
    return nullptr;
  }

  JunkParser_Base*
  JunkParser(x25519_keypair keys)
  {
    return new JunkDecrypter{keys};
  }

}  // namespace onionreq
