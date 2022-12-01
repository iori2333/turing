#pragma once
#include <Machine.h>
#include <Tape.h>

namespace turing::simulator {

using namespace machine;
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

constexpr auto RunInformationFormat =
    "Step   : {}\n"
    "State  : {}\n"
    "{}\n"
    "---------------------------------------------";

} // namespace constants

struct Simulator {
private:
  const Logger &logger;

  enum class Status {
    Running,
    Accepted,
    Stopped,
  };

  TuringState turingState;
  Symbols input;
  State currentState;
  Tapes tapes;
  int step;
  Status status;

  Simulator(TuringState state, SymbolsRef input)
      : turingState(std::move(state)), input(input),
        currentState(turingState.initialState), tapes(turingState, input),
        step(0), status(Status::Stopped), logger(Logger::instance()) {}

public:
  static auto of(TuringState state, SymbolsRef input) -> Result<Simulator> {
    const auto &logger = Logger::instance();

    for (auto verboseInfo = std::string{}; auto ch : input) {
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

  auto run() -> Result<> {
    status = Status::Running;
    while (status == Status::Running) {
      status = stepNext();
    }
    logger.info("Result: {}", tapes.result());
    logger.verbose(Logger::Level::Info,
                   "==================== END ====================");
    return status == Status::Accepted ? TuringError::Ok
                                      : TuringError::SimulatorNotAccepted;
  }

private:
  auto stepNext() -> Status {
    auto currentSymbols = tapes.read();
    auto inState = std::make_pair(currentState, currentSymbols);
    if (!turingState.transitions.contains(inState)) {
      return Status::Stopped;
    }
    auto [nextState, output, moves] = turingState.transitions.get(inState);
    tapes.write(output, moves);
    currentState = nextState;
    step++;
    if (turingState.finalStates.contains(currentState)) {
      return Status::Accepted;
    }
    logger.info(constants::RunInformationFormat, //
                step, currentState, tapes.toString());
    return Status::Running;
  }
};
} // namespace turing::simulator
