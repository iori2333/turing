#pragma once

#include <string>
#include <system_error>
#include <type_traits>

namespace turing::utils {
enum class TuringError : int {
  Ok = 0,
  ParserError,
  UnknownError,
};

using Error = std::error_code;

struct ErrorCategory final : public std::error_category {
  static auto instance() noexcept -> const ErrorCategory & {
    static ErrorCategory instance;
    return instance;
  }

  auto message(int ev) const -> std::string override {
    switch (static_cast<TuringError>(ev)) {
    case TuringError::Ok:
      return "Ok";
    default:
      return "Unknown error";
    }
  }
  auto name() const noexcept -> const char * override { return "turing"; }
};

// NOLINTNEXTLINE(readability-identifier-naming)
inline auto make_error_code(turing::utils::TuringError e) -> Error {
  return Error(static_cast<int>(e), ErrorCategory::instance());
}
} // namespace turing::utils

namespace std {
using turing::utils::TuringError;

template <> struct is_error_code_enum<TuringError> : true_type {};
} // namespace std

namespace turing::utils {
template <typename R> struct Result {
public:
  using ValueType = R;
  using ErrorType = Error;

  Result(ValueType value) : value(std::move(value)) {}
  Result(ErrorType error) : error(std::move(error)) {}
  template <typename E>
    requires std::is_error_code_enum_v<E>
  Result(E error) : error(std::make_error_code(error)) {}

  auto isOk() const -> bool { return bool(error); }
  auto isErr() const -> bool { return !isOk(); }

  auto unwrap() const -> ValueType { return value; }

  template <std::size_t n> constexpr auto get() const {
    if constexpr (n == 0) {
      return value;
    } else if constexpr (n == 1) {
      return error;
    } else {
      static_assert(n < 2, "Index out of bounds");
    }
  }

private:
  ValueType value;
  ErrorType error;
};
} // namespace turing::utils

namespace std {
using turing::utils::Result;

template <typename R>
struct tuple_size<Result<R>> : integral_constant<std::size_t, 2> {};

template <typename R> struct tuple_element<0, Result<R>> {
  using type = typename Result<R>::ValueType;
};

template <typename R> struct tuple_element<1, Result<R>> {
  using type = typename Result<R>::ErrorType;
};

template <std::size_t n, typename R> auto get(const Result<R> &result) {
  return result.template get<n>();
}
} // namespace std