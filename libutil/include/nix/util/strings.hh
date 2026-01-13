#pragma once

#include "nix/util/types.hh"

#include <list>
#include <set>
#include <string_view>
#include <string>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace nix {

/**
 * String tokenizer.
 *
 * See also `basicSplitString()`, which preserves empty strings between separators, as well as at the start and end.
 */
template<class C, class CharT = char>
C basicTokenizeString(std::basic_string_view<CharT> s, std::basic_string_view<CharT> separators);

/**
 * Like `basicTokenizeString` but specialized to the default `char`
 */
template<class C>
C tokenizeString(std::string_view s, std::string_view separators = " \t\n\r");

extern template std::list<std::string> tokenizeString(std::string_view s, std::string_view separators);
extern template StringSet tokenizeString(std::string_view s, std::string_view separators);
extern template std::vector<std::string> tokenizeString(std::string_view s, std::string_view separators);

/**
 * Split a string, preserving empty strings between separators, as well as at the start and end.
 *
 * Returns a non-empty collection of strings.
 */
template<class C, class CharT = char>
C basicSplitString(std::basic_string_view<CharT> s, std::basic_string_view<CharT> separators);

/**
 * Concatenate the given strings with a separator between the elements.
 */
template<class C>
std::string concatStringsSep(const std::string_view sep, const C & ss);

/**
 * Hash implementation that can be used for zero-copy heterogenous lookup from
 * P1690R1[1] in unordered containers.
 *
 * [1]: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1690r1.html
 */
struct StringViewHash
{
private:
    using HashType = std::hash<std::string_view>;

public:
    using is_transparent = void;

    auto operator()(const char * str) const
    {
        /* This has a slight overhead due to an implicit strlen, but there isn't
           a good way around it because the hash value of all overloads must be
           consistent. Delegating to string_view is the solution initially proposed
           in P0919R3. */
        return HashType{}(std::string_view{str});
    }

    auto operator()(std::string_view str) const
    {
        return HashType{}(str);
    }

    auto operator()(const std::string & str) const
    {
        return HashType{}(std::string_view{str});
    }
};

} // namespace nix
