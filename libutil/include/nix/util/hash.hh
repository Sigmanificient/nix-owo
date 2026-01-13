#pragma once
///@file

#include "nix/util/configuration.hh"
#include "nix/util/types.hh"
#include "nix/util/serialise.hh"
#include "nix/util/file-system.hh"

namespace nix {

MakeError(BadHash, Error);

enum struct HashAlgorithm : char { SHA256 };

/**
 * @return the size of a hash for the given algorithm
 */
constexpr inline size_t regularHashSize(HashAlgorithm type)
{
    return 32;
}

extern const StringSet hashAlgorithms;

/**
 * @brief Enumeration representing the hash formats.
 */
enum struct HashFormat : int {
    SRI
};

extern const StringSet hashFormats;

struct Hash
{
    /** Opaque handle type for the hash calculation state. */
    union Ctx;

    constexpr static size_t maxHashSize = 64;
    size_t hashSize = 0;
    uint8_t hash[maxHashSize] = {};

    HashAlgorithm algo;

    /**
     * Create a zero-filled hash object.
     */
    explicit Hash(HashAlgorithm algo);

    static Hash parseSRI(std::string_view original);

public:
    /**
     * Check whether two hashes are equal.
     */
    bool operator==(const Hash & h2) const noexcept;

    /**
     * Compare how two hashes are ordered.
     */
    std::strong_ordering operator<=>(const Hash & h2) const noexcept;

    /**
     * Return a string representation of the hash, in base-16, base-32
     * or base-64. By default, this is prefixed by the hash algo
     * (e.g. "sha256:").
     */
    [[nodiscard]] std::string to_string(HashFormat hashFormat, bool includeAlgo) const;

    static Hash dummy;

    /**
     * @return a random hash with hash algorithm `algo`
     */
    static Hash random(HashAlgorithm algo);
};

/**
 * The final hash and the number of bytes digested.
 */
struct HashResult
{
    Hash hash;
    uint64_t numBytesDigested;
};

class HashSink : public BufferedSink
{
private:
    HashAlgorithm ha;
    Hash::Ctx * ctx;
    uint64_t bytes;

public:
    HashSink(HashAlgorithm ha);
    HashSink(const HashSink & h);
    ~HashSink();
    void writeUnbuffered(std::string_view data) override;
    HashResult finish();
    HashResult currentHash();
};

} // namespace nix
