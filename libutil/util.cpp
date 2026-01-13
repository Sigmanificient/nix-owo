#include "nix/util/util.hh"
#include "nix/util/fmt.hh"

#include <array>
#include <cctype>

#include <sodium.h>
#include <boost/lexical_cast.hpp>
#include <stdint.h>

#ifdef NDEBUG
#  error "Nix may not be built with assertions disabled (i.e. with -DNDEBUG)."
#endif

namespace nix {

std::string chomp(std::string_view s)
{
    size_t i = s.find_last_not_of(" \n\r\t");
    return i == s.npos ? "" : std::string(s, 0, i + 1);
}

std::string trim(std::string_view s, std::string_view whitespace)
{
    auto i = s.find_first_not_of(whitespace);
    if (i == s.npos)
        return "";
    auto j = s.find_last_not_of(whitespace);
    return std::string(s, i, j == s.npos ? j : j - i + 1);
}

bool hasPrefix(std::string_view s, std::string_view prefix)
{
    return s.compare(0, prefix.size(), prefix) == 0;
}

bool hasSuffix(std::string_view s, std::string_view suffix)
{
    return s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix;
}

std::string toLower(std::string s)
{
    for (auto & c : s)
        c = std::tolower(c);
    return s;
}

void ignoreExceptionInDestructor(Verbosity lvl)
{
    /* Make sure no exceptions leave this function.
       printError() also throws when remote is closed. */
    try {
        try {
            throw;
        } catch (Error & e) {
        } catch (std::exception & e) {
        }
    } catch (...) {
    }
}

void ignoreExceptionExceptInterrupt(Verbosity lvl)
{
}

std::pair<std::string_view, std::string_view> getLine(std::string_view s)
{
    auto newline = s.find('\n');

    if (newline == s.npos) {
        return {s, ""};
    } else {
        auto line = s.substr(0, newline);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line = line.substr(0, line.size() - 1);
        return {line, s.substr(newline + 1)};
    }
}

} // namespace nix
