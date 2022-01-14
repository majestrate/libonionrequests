#pragma once
#include <array>
#include <string_view>

namespace onionreq::xed25519 {

using ustring_view = std::basic_string_view<unsigned char>;

/// XEd25519 signs a message (max length 256 bytes) given the curve25519 privkey and message.
std::array<unsigned char, 64> sign(
        ustring_view curve25519_privkey /* 32 bytes */,
        ustring_view msg /* <= 256 bytes */);

/// Verifies a curve25519 message allegedly signed by the given curve25519 pubkey
bool verify(
        ustring_view signature /* 64 bytes */,
        ustring_view curve25519_pubkey /* 32 bytes */,
        ustring_view msg /* <= 256 bytes */);

/// Returns the derived Ed25519 pubkey from a curve25519 pubkey
std::array<unsigned char, 32> pubkey(ustring_view curve25519_pubkey);

}
