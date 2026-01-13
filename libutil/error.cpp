#include <algorithm>

#include "nix/util/error.hh"
#include "nix/util/position.hh"

#include <cinttypes>
#include <iostream>
#include <optional>

namespace nix {

void BaseError::addTrace(std::shared_ptr<const Pos> && e, HintFmt hint, TracePrint print)
{
    err.traces.push_front(Trace{.pos = std::move(e), .hint = hint, .print = print});
}

std::optional<std::string> ErrorInfo::programName = std::nullopt;

std::ostream & operator<<(std::ostream & os, const HintFmt & hf)
{
    return os << hf.str();
}

/**
 * An arbitrarily defined value comparison for the purpose of using traces in the key of a sorted container.
 */
inline std::strong_ordering operator<=>(const Trace & lhs, const Trace & rhs)
{
    // `std::shared_ptr` does not have value semantics for its comparison
    // functions, so we need to check for nulls and compare the dereferenced
    // values here.
    if (lhs.pos != rhs.pos) {
        if (auto cmp = bool{lhs.pos} <=> bool{rhs.pos}; cmp != 0)
            return cmp;
        if (auto cmp = *lhs.pos <=> *rhs.pos; cmp != 0)
            return cmp;
    }
    // This formats a freshly formatted hint string and then throws it away, which
    // shouldn't be much of a problem because it only runs when pos is equal, and this function is
    // used for trace printing, which is infrequent.
    return lhs.hint.str() <=> rhs.hint.str();
}

/** Write to stderr in a robust and minimal way, considering that the process
 * may be in a bad state.
 */
static void writeErr(std::string_view buf)
{
    while (!buf.empty()) {
        auto n = write(STDERR_FILENO, buf.data(), buf.size());
        if (n < 0) {
            if (errno == EINTR)
                continue;
            abort();
        }
        buf = buf.substr(n);
    }
}

void panic(std::string_view msg)
{
    writeErr("\n\n" ANSI_RED "terminating due to unexpected unrecoverable internal error: " ANSI_NORMAL);
    writeErr(msg);
    writeErr("\n");
    std::terminate();
}

} // namespace nix
