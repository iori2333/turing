#pragma once
#include <string>
#include <unordered_set>
#include <vector>

namespace turing::machine {

using State = std::string;
using StateRef = std::string_view;
using StatesSet = std::unordered_set<State>;
using Symbol = char;
using SymbolSet = std::unordered_set<Symbol>;
using Size = std::size_t;

struct Transition {};
using Transitions = std::vector<Transition>;

struct TuringState {
  SymbolSet symbols;
  StatesSet states;
  SymbolSet tapeSymbols;
  State initialState;
  Symbol blankSymbol;
  StatesSet finalStates;
  Size tapeCount;
  Transitions transitions;
};
} // namespace turing::machine
