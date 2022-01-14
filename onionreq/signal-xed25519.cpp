// Wrappers around libsignal's XEd25519 code for dealing with X25519 -> Ed25519 code

#include "signal-xeddsa.h"
#include <cassert>
#include <stdexcept>
#include <sodium/randombytes.h>

extern "C" {
#include <xeddsa.h>
#include <crypto_additions.h>
}

namespace onionreq::xed25519 {


std::array<unsigned char, 64> sign(
        ustring_view curve25519_privkey,
        ustring_view msg) {

    assert(curve25519_privkey.size() == 64);
    assert(msg.size() <= 256);

    std::array<unsigned char, 64> random;
    randombytes_buf(random.data(), random.size());

    std::array<unsigned char, 64> sig;
    if (0 == xed25519_sign(sig.data(), curve25519_privkey.data(), msg.data(), msg.size(), random.data()))
        return sig;
    throw std::runtime_error("XEd25519 signature failed");
}

bool verify(ustring_view signature, ustring_view curve25519_pubkey, ustring_view msg) {
    return 0 == xed25519_verify(signature.data(), curve25519_pubkey.data(), msg.data(), msg.size());
}

std::array<unsigned char, 32> pubkey(ustring_view curve25519_pubkey) {
    if (!fe_isreduced(curve25519_pubkey.data()))
        throw std::runtime_error("XEd25519 signature derivation failed: curve pubkey is not reduced");

    fe u, y;
    fe_frombytes(u, curve25519_pubkey.data());
    fe_montx_to_edy(y, u);
    std::array<unsigned char, 32> ed_pubkey;
    fe_tobytes(ed_pubkey.data(), y);

    return ed_pubkey;
}


}
