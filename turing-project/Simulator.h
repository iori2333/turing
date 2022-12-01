#pragma once
#include <Machine.h>

namespace turing::simulator {

using machine::TuringState;
using utils::Error;
using utils::Logger;
using utils::Result;
using utils::TuringError;

namespace constants {

constexpr auto InvalidInputFormat =
    "Input: {}\n"
    "==================== ERR ====================\n"
    "error: '{}' was not declared in the set of input symbols\n"
    "Input: {}\n"
    "       {}\n"
    "==================== END ====================";

constexpr auto ValidInputFormat =
    "Input: {}\n"
    "==================== RUN ====================";

} // namespace constants

struct Simulator {
private:
  TuringState turingState;
  std::string_view input;
  const Logger &logger;

  Simulator(TuringState turingState, std::string_view input)
      : turingState(std::move(turingState)), input(input),
        logger(Logger::instance()) {}

public:
  static auto of(TuringState state, std::string_view input)
      -> Result<Simulator> {
    auto verboseInfo = std::string{};
    const auto &logger = Logger::instance();

    for (auto ch : input) {
      if (!state.symbols.contains(ch)) {
        verboseInfo += '^';
        logger.verbose(Logger::Level::Error, constants::InvalidInputFormat,
                       input, ch, input, verboseInfo);
        return Error(TuringError::SimulatorIllegalInput);
      }
      verboseInfo += ' ';
    }

    logger.verbose(Logger::Level::Info, constants::ValidInputFormat, input);
    return Simulator(std::move(state), input);
  }

  auto run() -> Result<> { return TuringError::Ok; }
};
} // namespace turing::simulator
