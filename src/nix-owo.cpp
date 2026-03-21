#include <atomic>
#include <cstdint>
#include <expected>
#include <iostream>
#include <mutex>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>

#include "utils.hpp"

namespace po = boost::program_options;

using namespace nix;

struct Params {
  bool show_help = false;

  std::string target_path;
  uint8_t jobs;
};

enum class State : uint8_t {
    FOUND_MATCH,
    FAILURE,
    ONGOING
};

static
std::expected<struct Params, std::string>
parse_arguments(int argc, char **argv)
{
  po::options_description desc("Available options");
  po::positional_options_description p_desc;
  po::variables_map vm;

  auto cpu_count = std::max(1U, std::thread::hardware_concurrency());

  p_desc.add("target", 1);
  desc.add_options()
      ("help,h", "show usage information")
      ("target", po::value<std::string>()->default_value("."), "Target path")
      ("jobs,j",
          po::value<uint8_t>()->default_value(1)->implicit_value(cpu_count),
          "number of parallel jobs to run")
      ;

  try {
      po::store(po::command_line_parser(argc, argv)
          .options(desc)
          .positional(p_desc)
          .run(),
          vm);
      po::notify(vm);

      if (vm.contains("help"))
            std::cout << "usage: nix-owo [TARGET = .] [OPTIONS]\n\n" << desc;

      return Params{
          .show_help = vm.contains("help"),
          .target_path = vm["target"].as<std::string>(),
          .jobs = vm["jobs"].as<uint8_t>(),
      };
  } catch (const std::exception &e) {
      return std::unexpected(e.what());
  }
}

static
bool check_for_readme(PosixSourceAccessor root, CanonPath &c_path)
{
    try {
        HashSink sink(HashAlgorithm::SHA256);
        if (!SAdumpPath(root, c_path, sink, 1)) {
            std::cerr << "Could not find README.md" << '\n';
            return false;
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;
}


int main(int argc, char **argv)
{
    std::expected<struct Params, std::string> res = parse_arguments(argc, argv);
    if (!res.has_value()) {
        std::cerr << res.error() << "\n";
        return EXIT_FAILURE;
    }

    struct Params &parameters = res.value();
    if (parameters.show_help)
        return EXIT_SUCCESS;

    std::filesystem::path abspath = absPath(parameters.target_path);
    PosixSourceAccessor root = abspath.root_path();

    std::vector<std::thread> threads;
    uint64_t num_threads = parameters.jobs;
    uint64_t interval = UINT64_MAX / num_threads;

    auto c_path = CanonPath{abspath.relative_path().string()};

    if (!check_for_readme(root, c_path)) {
        std::cerr << "Could not find README.md" << '\n';
        return EXIT_FAILURE;
    }

    std::mutex log_mutex;
    std::atomic<State> state = State::ONGOING;

    auto find_matching_hash = [&](uint64_t start, uint64_t end) {
        try {
            for (uint64_t num = start; num < end && state == State::ONGOING; ++num) {
                HashSink sink(HashAlgorithm::SHA256);
                SAdumpPath(root, c_path, sink, num);

                HashResult result = sink.finish();
                auto hash = result.hash.to_string(nix::HashFormat::SRI, true);

                if (hash[47] == '0' && hash[48] == 'w' && hash[49] == '0') {
                    std::lock_guard lock(log_mutex);
                    state = State::FOUND_MATCH;
                    std::cerr << '\n';
                    std::cout << hash << '\n';
                    std::cout << makeHeader(num);
                    return;
                }
                if (num % (100 * num_threads) == 0) {
                    std::lock_guard lock(log_mutex);
                    if (state != State::ONGOING)
                        return;
                    std::cerr << ".";
                }
            }
        } catch (std::exception &e) {
            std::lock_guard lock(log_mutex);
            state = State::FAILURE;
            std::cerr << '\n' << e.what() << '\n';
        }
    };

    for (uint64_t i = 1; i <= num_threads; ++i) {
        uint64_t start = 1 + (interval * (i - 1));
        uint64_t end = interval * i;
        threads.emplace_back(find_matching_hash, start, end);
    }

    for (auto &thread : threads) {
        try {
            thread.join();
        } catch (std::exception & e) {
            std::lock_guard lock(log_mutex);
            std::cerr << '\n' << e.what() << '\n';
            return EXIT_FAILURE;
        }
    }

    if (state != State::FOUND_MATCH) {
        std::cerr << "\nUnable to find match\n";
        return EXIT_FAILURE;
    }
}
