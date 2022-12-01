#pragma once
#include <iostream>

#include <StringUtils.h>

namespace turing::utils {

using concepts::StringConvertible;

#ifdef __turing_legacy__
#define __turing_string_convertible__
#else
#define __turing_string_convertible__ StringConvertible
#endif

struct Logger {
private:
  bool isVerbose;
  std::ostream &es;
  std::ostream &os;

  Logger() : isVerbose(false), es(std::cerr), os(std::cout) {}

public:
  enum struct Level { Info, Error };

  static auto instance() -> Logger & {
    static Logger logger;
    return logger;
  }

  auto setVerbose(bool verbose) -> void { isVerbose = verbose; }

  auto log(Level level, std::string_view message) const -> void {
    auto &stream = level == Level::Info ? os : es;
    stream << message << std::endl;
  }

  auto info(std::string_view fmt,
            __turing_string_convertible__ auto &&...args) const -> void {
    log(Level::Info, format(fmt, std::forward<decltype(args)>(args)...));
  }

  auto error(std::string_view fmt,
             __turing_string_convertible__ auto &&...args) const -> void {
    log(Level::Error, format(fmt, std::forward<decltype(args)>(args)...));
  }

  auto verbose(Level level, std::string_view fmt,
               __turing_string_convertible__ auto &&...args) const -> void {
    if (isVerbose) {
      log(level, format(fmt, std::forward<decltype(args)>(args)...));
    }
  }

  auto noVerbose(Level level, std::string_view fmt,
                 __turing_string_convertible__ auto &&...args) const -> void {
    if (!isVerbose) {
      log(level, format(fmt, std::forward<decltype(args)>(args)...));
    }
  }
};
} // namespace turing::utils
