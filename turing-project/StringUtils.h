#pragma once
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace std {
// NOLINTNEXTLINE(readability-identifier-naming)
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
concept Printable = requires(std::stringstream ss, T t) {
  { ss << t } -> std::convertible_to<std::ostream&>;
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
    -> std::vector<std::string_view> {
  auto ret = std::vector<std::string_view>{};
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

auto split(std::string_view ins, char delim, int max = -1)
    -> std::vector<std::string_view> {
  return split(ins, std::string_view{&delim, 1}, max);
}

auto replace(std::string_view ins, std::string_view from, std::string_view to)
    -> std::string {
  auto ret = std::string{};
  for (auto pos = ins.find(from); pos != std::string_view::npos;
       pos = ins.find(from)) {
    ret.append(ins.substr(0, pos));
    ret.append(to);
    ins.remove_prefix(pos + from.size());
  }
  ret.append(ins);
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

auto trim(std::string_view s, char symbol = ' ') -> std::string_view {
  auto start = s.find_first_not_of(symbol);
  if (start == std::string_view::npos) {
    return s;
  }
  auto end = s.find_last_not_of(symbol);
  return s.substr(start, end - start + 1);
}

} // namespace turing::utils
