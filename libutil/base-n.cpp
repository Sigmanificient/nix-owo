#include <string_view>

#include "nix/util/array-from-string-literal.hh"
#include "nix/util/util.hh"
#include "nix/util/base-n.hh"

using namespace std::literals;

namespace nix {

constexpr static const std::array<char, 16> base16Chars = "0123456789abcdef"_arrayNoNull;

std::string base16::encode(std::span<const std::byte> b)
{
    std::string buf;
    buf.reserve(b.size() * 2);
    for (size_t i = 0; i < b.size(); i++) {
        buf.push_back(base16Chars[(uint8_t) b.data()[i] >> 4]);
        buf.push_back(base16Chars[(uint8_t) b.data()[i] & 0x0f]);
    }
    return buf;
}

constexpr static const std::array<char, 64> base64Chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"_arrayNoNull;

std::string base64::encode(std::span<const std::byte> s)
{
    std::string res;
    res.reserve((s.size() + 2) / 3 * 4);
    int data = 0, nbits = 0;

    for (std::byte c : s) {
        data = data << 8 | (uint8_t) c;
        nbits += 8;
        while (nbits >= 6) {
            nbits -= 6;
            res.push_back(base64Chars[data >> nbits & 0x3f]);
        }
    }

    if (nbits)
        res.push_back(base64Chars[data << (6 - nbits) & 0x3f]);
    while (res.size() % 4)
        res.push_back('=');

    return res;
}

} // namespace nix
