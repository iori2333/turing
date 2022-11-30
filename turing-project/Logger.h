#pragma once
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

namespace std {
// NOLINTNEXTLINE
auto to_string(string_view s) -> string { return string(s); }
} // namespace std

namespace turing::utils {

namespace details {
// clang-format off
template <typename T>
concept ToStringConvertible = requires(T t) {
  { std::to_string(t) } -> std::convertible_to<std::string>;
};

template <typename T>
concept Printable = requires(T t) {
  { std::ostream{} << t } -> std::convertible_to<std::ostream&>;
};

template <typename T>
concept IsString = std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, const char*>;

template <typename T>
concept StringConvertible = ToStringConvertible<T> || IsString<T> || Printable<T>;
// clang-format on

template <StringConvertible T> auto toString(T &&arg) -> std::string {
  if constexpr (IsString<T>) {
    return arg;
  } else if constexpr (ToStringConvertible<T>) {
    return std::to_string(std::forward<T>(arg));
  } else {
    std::ostringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
  }
}
} // namespace details

auto split(std::string_view ins, std::string_view delim = " ", int max = -1)
    -> std::vector<std::string> {
  auto ret = std::vector<std::string>{};
  for (auto pos = ins.find(delim); pos != std::string_view::npos;
       pos = ins.find(delim)) {
    ret.emplace_back(ins.substr(0, pos));
    ins.remove_prefix(pos + delim.size());
    if (max > 0 && ret.size() == max - 1) {
      break;
    }
  }
  ret.emplace_back(ins);
  return ret;
}

template <typename... Args>
auto format(std::string_view fmt, Args &&...args) -> std::string {
  auto is = std::ostringstream{};
  auto tupArgs = std::make_tuple(std::forward<Args>(args)...);
  auto vecArgs = std::vector<std::string>{};

  std::apply(
      [&vecArgs](auto &&...args) {
        (vecArgs.emplace_back(details::toString(args)), ...);
      },
      tupArgs);

  auto fmtVec = split(fmt, "{}");
  auto fmtSize = fmtVec.size();
  auto argSize = sizeof...(args);
  for (auto i = 0; i < fmtSize; ++i) {
    is << fmtVec[i];
    if (i < argSize && i != fmtSize - 1) {
      is << vecArgs[i];
    }
  }
  return is.str();
}

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
