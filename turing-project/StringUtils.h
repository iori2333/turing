#pragma once
#include <algorithm>
#include <regex>
#include <sstream>
#include <tuple>
#include <vector>

#ifdef __turing_legacy__
namespace std {
template <typename _From, typename _To>
concept convertible_to = is_convertible_v<_From, _To> &&
                         requires { static_cast<_To>(std::declval<_From>()); };
}
#endif

namespace turing::utils {

namespace concepts {
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
concept Printable = requires(std::stringstream &ss, T t) {
  { ss << t } -> std::convertible_to<std::ostream &>;
};

template <typename T>
concept IsString = std::is_same_v<std::decay_t<T>, std::string>;

template <typename T>
concept StringConstructable = std::is_constructible_v<std::string, T>;

template<typename T>
concept IsCharacter = std::is_same_v<std::decay_t<T>, char>;

template <typename T>
concept StringConvertible = StdToStringConvertible<T> ||
    ToStringConvertible<T> || Printable<T> ||
    IsString<T> || StringConstructable<T> || IsCharacter<T>;

template <typename V>
concept Iterable = requires(V &v) {
  { v.begin() } -> std::convertible_to<typename V::iterator>;
  { v.end() } -> std::convertible_to<typename V::iterator>;
};

template<typename V>
concept ConstIterable = requires(const V &v) {
  { v.begin() } -> std::convertible_to<typename V::const_iterator>;
  { v.end() } -> std::convertible_to<typename V::const_iterator>;
};
// clang-format on
} // namespace concepts

using concepts::ConstIterable;
using concepts::Iterable;
using concepts::StringConvertible;

using svmatch = std::match_results<std::string_view::const_iterator>;

template <StringConvertible T> inline auto toString(T &&arg) -> std::string {
  if constexpr (concepts::IsString<T>) {
    return arg;
  } else if constexpr (concepts::IsCharacter<T>) {
    return std::string(1, arg);
  } else if constexpr (concepts::StringConstructable<T>) {
    return std::string(arg);
  } else if constexpr (concepts::StdToStringConvertible<T>) {
    return std::to_string(arg);
  } else if constexpr (concepts::ToStringConvertible<T>) {
    return arg.toString();
  } else {
    std::ostringstream oss;
    oss << std::forward<decltype(arg)>(arg);
    return oss.str();
  }
}

inline auto split(std::string_view ins, std::string_view delim = " ",
                  int max = -1) -> std::vector<std::string_view> {
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

inline auto split(std::string_view ins, char delim, int max = -1)
    -> std::vector<std::string_view> {
  return split(ins, std::string_view{&delim, 1}, max);
}

inline auto omitEmpty(std::vector<std::string_view> &vec)
    -> std::vector<std::string_view> {
  std::erase_if(vec, [](auto s) { return s.empty(); });
  return vec;
}

inline auto replace(std::string_view ins, std::string_view from,
                    std::string_view to) -> std::string {
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

template <ConstIterable V>
  requires StringConvertible<typename V::value_type>
auto join(const V &vec, std::string_view delim = " ") -> std::string {
  auto ret = std::string{};

  for (auto it = vec.begin(); it != vec.end();) {
    ret.append(toString(*it));
    if (++it != vec.end()) {
      ret.append(delim);
    }
  }
  return ret;
}

template <ConstIterable V>
  requires StringConvertible<typename V::value_type>
inline auto join(const V &vec, char delim) -> std::string {
  return join(vec, std::string_view(&delim, 1));
}

template <StringConvertible... Args>
auto format(std::string_view fmt, Args &&...args) -> std::string {
  if constexpr (sizeof...(args) == 0) {
    return std::string{fmt};
  }

  auto is = std::ostringstream{};
  auto tupArgs = std::make_tuple(std::forward<Args>(args)...);
  auto vecArgs = std::vector<std::string>{};

  std::apply(
      [&vecArgs](auto &&...args) {
        (vecArgs.emplace_back(toString(args)), ...);
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

inline auto trim(std::string_view s, char symbol = ' ') -> std::string_view {
  auto start = s.find_first_not_of(symbol);
  if (start == std::string_view::npos) {
    return s;
  }
  auto end = s.find_last_not_of(symbol);
  return s.substr(start, end - start + 1);
}

template <Iterable V>
  requires concepts::IsString<typename V::value_type>
inline auto alignRight(V &strings, char symbol = ' ',
                       std::size_t size = std::string::npos)
    -> decltype(strings) {
  if (size == std::string::npos) {
    size = std::max_element(strings.begin(), strings.end(),
                            [](const auto &i1, const auto &i2) {
                              return i1.size() < i2.size();
                            })
               ->size();
  }
  for (auto &s : strings) {
    if (s.size() < size) {
      for (auto i = 0; i < size - s.size(); ++i) {
        s.push_back(symbol);
      }
    }
  }
  return strings;
}

} // namespace turing::utils
