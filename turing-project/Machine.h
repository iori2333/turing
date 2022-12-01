#pragma once
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace turing::machine {

using Position = int;
enum class Move : Position { Left = -1, Right = 1, Stay = 0 };

using State = std::string;
using StateRef = std::string_view;
using StatesSet = std::unordered_set<State>;
using Symbol = char;
using SymbolSet = std::unordered_set<Symbol>;
using Symbols = std::basic_string<Symbol>;
using SymbolsRef = std::basic_string_view<Symbol>;
using Size = std::size_t;

struct Transition {
private:
  State curr;
  Symbols input;

  State next;
  Symbols output;
  std::vector<Move> moves;

public:
  Transition(StateRef curr, SymbolsRef input, StateRef next, SymbolsRef output,
             std::vector<Move> moves)
      : curr(curr), input(input), next(next), output(output),
        moves(std::move(moves)) {}

  using StateInput = std::pair<State, Symbols>;
  using StateOutput = std::tuple<State, Symbols, std::vector<Move>>;

  auto states() && -> std::pair<StateInput, StateOutput> {
    return {{std::move(curr), std::move(input)},
            {std::move(next), std::move(output), std::move(moves)}};
  }

  auto states() const & -> std::pair<StateInput, StateOutput> {
    return {{curr, input}, {next, output, moves}};
  }

  auto isStarTransition() const -> bool {
    return input.find('*') != std::string::npos ||
           output.find('*') != std::string::npos;
  }

  auto convertTransitions() -> std::vector<Transition> { return {}; }
};

struct Transitions {
public:
  using TransitionMap =
      std::map<Transition::StateInput, Transition::StateOutput>;
  using iterator = typename TransitionMap::iterator;
  using const_iterator = typename TransitionMap::const_iterator;
  using value_type = typename TransitionMap::value_type;

private:
  TransitionMap transitions;

public:
  auto insert(Transition &&transition) -> void {
    transitions.insert(std::move(transition).states());
  }

  auto insert(const Transition &transition) -> void {
    transitions.insert(transition.states());
  }

  auto erase(const Transition::StateInput &stateInput) -> void {
    transitions.erase(stateInput);
  }

  auto find(const Transition::StateInput &stateInput) const
      -> std::optional<Transition::StateOutput> {
    auto it = transitions.find(stateInput);
    return it == transitions.end() ? std::nullopt : std::optional(it->second);
  }

  auto contains(const Transition::StateInput &stateInput) const -> bool {
    return transitions.contains(stateInput);
  }

  auto size() const -> Size { return transitions.size(); }

  auto begin() -> iterator { return transitions.begin(); }

  auto begin() const -> const_iterator { return transitions.begin(); }

  auto end() -> iterator { return transitions.end(); }

  auto end() const -> const_iterator { return transitions.end(); }

  auto toString() const -> std::string {
    auto os = std::vector<std::string>{};
    std::transform( //
        transitions.begin(), transitions.end(), std::back_inserter(os),
        [](const auto &v) {
          auto ret = std::string{};
          const auto &[in, out] = v;
          const auto &[curr, input] = in;
          const auto &[next, output, moves] = out;
          ret += "    " + curr + ' ' + input + ' ' + next + ' ' + output + ' ';
          for (auto move : moves) {
            switch (move) {
            case Move::Left:
              ret += 'l';
              break;
            case Move::Right:
              ret += 'r';
              break;
            case Move::Stay:
              ret += '*';
              break;
            }
          }
          return ret;
        });
    return utils::join(os, '\n');
  }
};

struct TuringState {
  SymbolSet symbols;
  StatesSet states;
  SymbolSet tapeSymbols;
  State initialState;
  Symbol blankSymbol;
  StatesSet finalStates;
  Size tapeCount;
  Transitions transitions;

  static constexpr auto FormatTemplate = "TuringState {\n"
                                         "  symbols: {}\n"
                                         "  states: {}\n"
                                         "  tapeSymbols: {}\n"
                                         "  initialState: {}\n"
                                         "  blankSymbol: {}\n"
                                         "  finalStates: {}\n"
                                         "  tapeCount: {}\n"
                                         "  transitions:\n{}\n"
                                         "  totalTransitions: {}\n"
                                         "}";

  auto toString() -> std::string {
    return utils::format(FormatTemplate, utils::join(symbols),
                         utils::join(states), utils::join(tapeSymbols),
                         initialState, blankSymbol, utils::join(finalStates),
                         tapeCount, transitions, transitions.size());
  }
};
} // namespace turing::machine
