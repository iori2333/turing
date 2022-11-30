#pragma once
#include <Machine.h>

namespace turing::simulator {

using machine::TuringState;
using utils::Error;
using utils::Result;
using utils::TuringError;

struct Simulator {

private:
  TuringState turingState;
  Simulator(TuringState turingState) : turingState(std::move(turingState)) {}

public:
  static auto of(TuringState state) { return Simulator(std::move(state)); }
  auto run() -> Result<> { return TuringError::Ok; }
};
} // namespace turing::simulator
