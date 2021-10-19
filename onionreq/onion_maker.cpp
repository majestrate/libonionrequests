#include "onion_maker.hpp"
#include "key_types.hpp"
#include "channel_encryption.hpp"
#include <oxenc/base64.h>
#include <variant>
#include "junk.hpp"

#include <sodium/crypto_box.h>
#include <spdlog/spdlog.h>

namespace onionreq
{
  namespace
  {
    std::string
    encode_size(uint32_t s)
    {
      std::string str{reinterpret_cast<const char*>(&s), 4};
#if __BYTE_ORDER == __BIG_ENDIAN
      std::swap(str[0], str[3]);
      std::swap(str[1], str[2]);
#elif __BYTE_ORDER != __LITTLE_ENDIAN
#error Unknown endianness
#endif
      return str;
    }

    std::pair<std::shared_ptr<JunkParser_Base>, std::string>
    MakeJunk(
        const RemoteResource_t& remote,
        std::string_view plain,
        EncryptType keytype = EncryptType::aes_gcm)
    {
      if (auto* ptr = std::get_if<SNodeInfo>(&remote))
        return {std::shared_ptr<JunkParser_Base>(JunkParser(*ptr, keytype)), std::string{plain}};

      x25519_pubkey pubkey = std::visit([](auto&& k) { return k.onion; }, remote);

      x25519_keypair _keys{};
      crypto_box_keypair(_keys.first.data(), _keys.second.data());

      ChannelEncryption enc{_keys.second, _keys.first, false};

      std::string ct = encode_size(plain.size()) + enc.encrypt(keytype, plain, pubkey)
          + nlohmann::json{{"ephemeral_key", _keys.first.hex()}, {"enc_type", to_string(keytype)}}
                .dump();
      return {std::shared_ptr<JunkParser_Base>(JunkParser(_keys)), ct};
    }

  }  // namespace

  class OnionMaker_Impl : public OnionMaker_Base
  {
   protected:
    virtual EncryptType
    EncType() const = 0;

   public:
    OnionPayload
    MakeOnion(std::string_view payload, const OnionPath& path) const override
    {
      const auto control_json =
          std::visit([](const auto& val) { return val.ControlData(); }, path.remote);
      const auto control = control_json.dump();
      const auto keytype = EncType();

      OnionPayload onion{};
      onion.path = path;
      auto& blob = onion.ciphertext;

      // Ephemeral keypair:
      x25519_pubkey A;
      x25519_seckey a;
      x25519_pubkey final_pubkey;
      x25519_seckey final_seckey;

      auto it = path.hops.rbegin();
      {
        crypto_box_keypair(A.data(), a.data());
        ChannelEncryption e{a, A, false};

        auto data = encode_size(payload.size());
        data += payload;
        data += control;
        blob = e.encrypt(keytype, data, path.hops.back().onion);
        // Save these because we need them again to decrypt the final response:
        final_seckey = a;
        final_pubkey = A;
      }

      for (it++; it != path.hops.rend(); it++)
      {
        // Routing data for this hop:
        nlohmann::json routing{
            {"destination", std::prev(it)->identity.hex()},  // Next hop's ed25519 key
            {"ephemeral_key", A.hex()},  // The x25519 ephemeral_key here is the key for the
                                         // *next* hop to use
            {"enc_type", to_string(keytype)},
        };

        blob = encode_size(blob.size()) + blob + routing.dump();

        // Generate eph key for *this* request and encrypt it:
        crypto_box_keypair(A.data(), a.data());
        ChannelEncryption e{a, A, false};
        blob = e.encrypt(keytype, blob, it->onion);
      }

      // The data going to the first hop needs to be wrapped in one more layer to tell the
      // first hop how to decrypt the initial payload:
      blob = encode_size(blob.size()) + blob
          + nlohmann::json{{"ephemeral_key", A.hex()}, {"enc_type", to_string(keytype)}}.dump();

      onion.maybeDecryptResponse =
          [keytype,
           d = ChannelEncryption{final_seckey, final_pubkey, false},
           finalKey = path.hops.back().onion](std::string ct) -> std::optional<std::string> {
        auto body = ct;
        auto orig_size = ct.size();
        try
        {
          return d.decrypt(keytype, body, finalKey);
        }
        catch (...)
        {}

        if (oxenc::is_base64(body))
        {
          body = oxenc::from_base64(body);
          try
          {
            return d.decrypt(keytype, body, finalKey);
          }
          catch (...)
          {}
        }
        return std::nullopt;
      };
      return onion;
    }
  };

  class OnionMaker_ChaCha : public OnionMaker_Impl
  {
   protected:
    EncryptType
    EncType() const override
    {
      return EncryptType::xchacha20;
    }
  };

  class OnionMaker_AESGCM : public OnionMaker_Impl
  {
   protected:
    EncryptType
    EncType() const override
    {
      return EncryptType::aes_gcm;
    }
  };

  OnionMaker_Base* OnionMaker(all_aesgcm_hops)
  {
    return new OnionMaker_AESGCM{};
  }

  OnionMaker_Base* OnionMaker(all_xchacha20_hops)
  {
    return new OnionMaker_ChaCha{};
  }
}  // namespace onionreq
