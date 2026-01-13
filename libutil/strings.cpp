#include "nix/util/strings-inline.hh"
#include "nix/util/os-string.hh"
#include "nix/util/error.hh"

namespace nix {

template std::list<std::string> tokenizeString(std::string_view s, std::string_view separators);
template StringSet tokenizeString(std::string_view s, std::string_view separators);
template std::vector<std::string> tokenizeString(std::string_view s, std::string_view separators);

template std::list<OsString>
basicSplitString(std::basic_string_view<OsChar> s, std::basic_string_view<OsChar> separators);

template std::string concatStringsSep(std::string_view, const std::list<std::string> &);
template std::string concatStringsSep(std::string_view, const StringSet &);
template std::string concatStringsSep(std::string_view, const std::vector<std::string> &);
template std::string concatStringsSep(std::string_view, const boost::container::small_vector<std::string, 64> &);

typedef std::string_view strings_2[2];
template std::string concatStringsSep(std::string_view, const strings_2 &);
typedef std::string_view strings_3[3];
template std::string concatStringsSep(std::string_view, const strings_3 &);
typedef std::string_view strings_4[4];
template std::string concatStringsSep(std::string_view, const strings_4 &);
} // namespace nix
