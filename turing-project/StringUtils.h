#pragma once
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace turing::utils {

namespace details {
// clang-format off
template <typename T>
concept StdToStringConvertible = requires(T t) {
  { std::to_string(t) } -> std::convertible_to<std::string>;
};

template <typename T>
concept ToStringConvertible = requires(T t) {
  { t.toString() } -> std::convertible_to<std::string>;
};

template <typename T>
concept Printable = requires(std::stringstream ss, T t) {
  { ss << t } -> std::convertible_to<std::ostream&>;
};

template <typename T>
concept IsString = std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, const char*>;

template <typename T>
concept StringConstructable = std::is_same_v<std::decay_t<T>, std::string_view>;

template<typename T>
concept IsCharacter = std::is_same_v<std::decay_t<T>, char>;

template <typename T>
concept StringConvertible = StdToStringConvertible<T> ||
    ToStringConvertible<T> || Printable<T> ||
    IsString<T> || StringConstructable<T> || IsCharacter<T>;

template <typename V>
concept Iterable = requires(V v) {
  { v.begin() } -> std::convertible_to<typename V::iterator>;
  { v.end() } -> std::convertible_to<typename V::iterator>;
};
// clang-format on

template <StringConvertible T> inline auto toString(T &&arg) -> std::string {
  if constexpr (IsString<T>) {
    return arg;
  } else if constexpr (IsCharacter<T>) {
    return std::string(1, arg);
  } else if constexpr (StringConstructable<T>) {
    return std::string(arg);
  } else if constexpr (StdToStringConvertible<T>) {
    return std::to_string(arg);
  } else if constexpr (ToStringConvertible<T>) {
    return arg.toString();
  } else {
    std::ostringstream oss;
    oss << std::forward<decltype(arg)>(arg);
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

auto omitEmpty(std::vector<std::string_view> &vec)
    -> std::vector<std::string_view> {
  std::erase_if(vec, [](auto s) { return s.empty(); });
  return vec;
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

template <details::Iterable V>
  requires details::StringConvertible<typename V::value_type>
auto join(const V &vec, std::string_view delim) -> std::string {
  auto ret = std::string{};

  for (auto it = vec.begin(); it != vec.end();) {
    ret.append(details::toString(*it));
    if (++it != vec.end()) {
      ret.append(delim);
    }
  }
  return ret;
}

template <details::Iterable V>
  requires details::StringConvertible<typename V::value_type>
auto join(const V &vec, char delim = ' ') -> std::string {
  return join(vec, std::string_view(&delim, 1));
}

template <typename... Args>
auto format(std::string_view fmt, Args &&...args) -> std::string {
  if constexpr (sizeof...(args) == 0) {
    return std::string{fmt};
  }

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
