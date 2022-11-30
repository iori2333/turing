#pragma once
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

#include <StringUtils.h>

namespace turing::utils {
struct Logger {
private:
  bool verbose;

  Logger() : verbose(false) {
    std::cout.sync_with_stdio(false);
    std::cerr.sync_with_stdio(false);
  }

public:
  static auto instance() -> Logger & {
    static Logger logger;
    return logger;
  }

  auto setVerbose(bool doVerbose) -> void { verbose = doVerbose; }

  auto info(std::string_view message) const -> void {
    std::cout << message << std::endl;
  }

  auto error(std::string_view message) const -> void {
    if (verbose) {
      std::cerr << message << std::endl;
    }
  }

  template <typename... Args>
  auto infof(std::string_view fmt, Args &&...args) const -> void {
    info(format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  auto errorf(std::string_view fmt, Args &&...args) const -> void {
    if (verbose) {
      std::cerr << format(fmt, std::forward<Args>(args)...) << std::endl;
    }
  }
};
} // namespace turing::utils
