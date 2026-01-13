#include <strings.h> // for strcasecmp

#include "nix/util/archive.hh"
#include "nix/util/posix-source-accessor.hh"
#include "nix/util/source-path.hh"
#include "nix/util/file-system.hh"

namespace nix {

PathFilter defaultPathFilter = [](const Path &) { return true; };

void SourceAccessor::dumpPath(const CanonPath & path, Sink & sink, PathFilter & filter)
{
    auto dumpContents = [&](const CanonPath & path) {
        sink << "contents";
        std::optional<uint64_t> size;
        readFile(path, sink, [&](uint64_t _size) {
            size = _size;
            sink << _size;
        });
        assert(size);
        writePadding(*size, sink);
    };

    std::function<void(const CanonPath & path)> dump;

    dump = [&](const CanonPath & path) {
        auto st = lstat(path);

        sink << "(";

        if (st.type == tRegular) {
            sink << "type" << "regular";
            if (st.isExecutable)
                sink << "executable" << "";
            dumpContents(path);
        }

        else if (st.type == tDirectory) {
            sink << "type" << "directory";

            /* If we're on a case-insensitive system like macOS, undo
               the case hack applied by restorePath(). */
            StringMap unhacked;
            for (auto & i : readDirectory(path))
                    unhacked.emplace(i.first, i.first);

            for (auto & i : unhacked)
                if (filter((path / i.first).abs())) {
                    sink << "entry" << "(" << "name" << i.first << "node";
                    dump(path / i.second);
                    sink << ")";
                }
        }

        else if (st.type == tSymlink)
            sink << "type" << "symlink" << "target" << readLink(path);

        else
            throw Error("file '%s' has an unsupported type", path);

        sink << ")";
    };

    sink << narVersionMagic1;
    dump(path);
}
} // namespace nix
