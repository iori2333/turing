#pragma once
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>

namespace turing::utils {
enum struct TuringError : int {
  Ok = 0,
  ParserInvalidStates,
  ParserInvalidSymbols,
  ParserInvalidTapeSymbols,
  ParserInvalidInitialState,
  ParserInvalidBlankSymbol,
  ParserInvalidFinalStates,
  ParserInvalidTapeCount,
  ParserInvalidTransition,
  ParserDuplicateDefinition,
  SimulatorIllegalInput,
  SimulatorNotAccepted,
  UnknownError
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
    case TuringError::ParserInvalidSymbols:
    case TuringError::ParserInvalidTapeSymbols:
    case TuringError::ParserInvalidInitialState:
    case TuringError::ParserInvalidBlankSymbol:
    case TuringError::ParserInvalidFinalStates:
    case TuringError::ParserInvalidTapeCount:
    case TuringError::ParserInvalidTransition:
    case TuringError::ParserDuplicateDefinition:
    case TuringError::ParserInvalidStates:
      return "syntax error";
    case TuringError::SimulatorIllegalInput:
      return "illegal input";
    case TuringError::SimulatorNotAccepted:
      return "not accepted";
    default:
      return "unknown error";
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
template <typename R = void> struct Result;

template <typename R> struct Result {
public:
  using ValueType = R;
  using ErrorType = Error;

  Result(ValueType value) : inner(std::move(value)) {}
  Result(ErrorType error) : inner(std::move(error)) {}
  Result(TuringError error) : inner(error) {}

  auto isOk() const -> bool { return std::holds_alternative<ValueType>(inner); }
  auto isErr() const -> bool { return !isOk(); }

  auto unwrap() const -> ValueType & { return std::get<ValueType>(inner); }
  auto onError(std::function<void(const Error &e)> fn) const -> ValueType & {
    if (isErr()) {
      fn(error());
      throw std::runtime_error("unreachable");
    }
    return unwrap();
  }
  auto error() const -> ErrorType { return std::get<ErrorType>(inner); }

  auto operator*() const -> ValueType & { return unwrap(); }

  operator bool() const { return isOk(); }

private:
  mutable std::variant<ValueType, ErrorType> inner;
};

template <> struct Result<void> {
public:
  using ValueType = void;
  using ErrorType = Error;

  Result(ErrorType error) : inner(std::move(error)) {}
  Result(TuringError error) : inner(error) {}
  Result() : Result(TuringError::Ok) {}

  auto isOk() const -> bool { return inner == TuringError::Ok; }
  auto isErr() const -> bool { return !isOk(); }

  auto onError(std::function<void(const Error &e)> fn) const -> ValueType {
    if (isErr()) {
      fn(error());
    }
  }
  auto error() const -> ErrorType { return inner; }

  operator bool() const { return isOk(); }

private:
  Error inner;
};
} // namespace turing::utils
