#include <cstring>

#include <openssl/crypto.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include "nix/util/hash.hh"
#include "nix/util/base-n.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sodium.h>

namespace nix {

const StringSet hashAlgorithms = {"sha256"};

const StringSet hashFormats = {"sri"};

Hash::Hash(HashAlgorithm algo)
    : algo(algo)
{
    hashSize = regularHashSize(algo);
    assert(hashSize <= maxHashSize);
    memset(hash, 0, maxHashSize);
}

bool Hash::operator==(const Hash & h2) const noexcept
{
    if (hashSize != h2.hashSize)
        return false;
    for (unsigned int i = 0; i < hashSize; i++)
        if (hash[i] != h2.hash[i])
            return false;
    return true;
}

std::strong_ordering Hash::operator<=>(const Hash & h) const noexcept
{
    if (auto cmp = hashSize <=> h.hashSize; cmp != 0)
        return cmp;
    for (unsigned int i = 0; i < hashSize; i++) {
        if (auto cmp = hash[i] <=> h.hash[i]; cmp != 0)
            return cmp;
    }
    if (auto cmp = algo <=> h.algo; cmp != 0)
        return cmp;
    return std::strong_ordering::equivalent;
}

std::string Hash::to_string(HashFormat hashFormat, bool includeAlgo) const
{
    std::string s = "sha256-";
    const auto bytes = std::as_bytes(std::span<const uint8_t>{&hash[0], hashSize});
    assert(hashSize);
    s += base64::encode(bytes);
    return s;
}

Hash Hash::dummy(HashAlgorithm::SHA256);

namespace {

/// Private convenience
struct DecodeNamePair
{
    decltype(base16::decode) * decode;
    std::string_view encodingName;
};

} // namespace

Hash Hash::random(HashAlgorithm algo)
{
    Hash hash(algo);
    randombytes_buf(hash.hash, hash.hashSize);
    return hash;
}

union Hash::Ctx
{
    SHA256_CTX sha256;
};

static void start(HashAlgorithm ha, Hash::Ctx & ctx)
{
    SHA256_Init(&ctx.sha256);
}

static void update(HashAlgorithm ha, Hash::Ctx & ctx, std::string_view data)
{
    SHA256_Update(&ctx.sha256, data.data(), data.size());
}

static void finish(HashAlgorithm ha, Hash::Ctx & ctx, unsigned char * hash)
{
    SHA256_Final(hash, &ctx.sha256);
}

Hash hashString(HashAlgorithm ha, std::string_view s)
{
    Hash::Ctx ctx;
    Hash hash(ha);
    start(ha, ctx);
    update(ha, ctx, s);
    finish(ha, ctx, hash.hash);
    return hash;
}

Hash hashFile(HashAlgorithm ha, const Path & path)
{
    HashSink sink(ha);
    readFile(path, sink);
    return sink.finish().hash;
}

HashSink::HashSink(HashAlgorithm ha)
    : ha(ha)
{
    ctx = new Hash::Ctx;
    bytes = 0;
    start(ha, *ctx);
}

HashSink::~HashSink()
{
    bufPos = 0;
    delete ctx;
}

void HashSink::writeUnbuffered(std::string_view data)
{
    bytes += data.size();
    update(ha, *ctx, data);
}

HashResult HashSink::finish()
{
    flush();
    Hash hash(ha);
    nix::finish(ha, *ctx, hash.hash);
    return HashResult(hash, bytes);
}

HashResult HashSink::currentHash()
{
    flush();
    Hash::Ctx ctx2 = *ctx;
    Hash hash(ha);
    nix::finish(ha, ctx2, hash.hash);
    return HashResult(hash, bytes);
}

Hash compressHash(const Hash & hash, unsigned int newSize)
{
    Hash h(hash.algo);
    h.hashSize = newSize;
    for (unsigned int i = 0; i < hash.hashSize; ++i)
        h.hash[i % newSize] ^= hash.hash[i];
    return h;
}

} // namespace nix
