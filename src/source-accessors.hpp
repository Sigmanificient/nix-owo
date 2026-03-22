#include <nix/util/base-n.hh>
#include <nix/util/hash.hh>
#include <nix/util/posix-source-accessor.hh>

struct MagicSourceAccessor : public nix::PosixSourceAccessor {
  protected:
    uint64_t m_magic = 0;

    const std::string m_base;

  public:
    MagicSourceAccessor(const std::filesystem::path& base)
      : nix::PosixSourceAccessor(base.root_path()),
      m_base(base.c_str())
    {}

    void setMagicNumber(uint64_t num) { m_magic = num; }

    void readFile(
        const nix::CanonPath &path,
        nix::Sink &sink,
        std::function<void(uint64_t)> callback
    ) override {
        std::string s = readFile(path);
        uint64_t size = s.size();

        callback(size);
        sink(s);
    }

    std::string readFile(const nix::CanonPath &path) override {
        const std::string &abspath = path.abs();
        std::string content = nix::readFile(abspath);

        if (path.abs() == m_base + "/README.md")
            content += "<!-- " + std::to_string(m_magic) + " -->\n";
        return content;
    };
};
