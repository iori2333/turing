#pragma once
#include <map>
#include <queue>
#include <set>

#include <StringUtils.h>

namespace turing::machine {

using Size = std::size_t;
using Position = int;

enum class Move : Position { Left = -1, Right = 1, Stay = 0 };
using Moves = std::vector<Move>;
using MovesRef = const std::vector<Move> &;

using State = std::string;
using StateRef = std::string_view;
using StatesSet = std::set<State>;

using Symbol = char;
using SymbolSet = std::set<Symbol>;
using Symbols = std::basic_string<Symbol>;
using SymbolsRef = std::basic_string_view<Symbol>;

struct TuringState;

struct Transition {
private:
  State curr;
  Symbols input;

  State next;
  Symbols output;
  Moves moves;

public:
  Transition(StateRef curr, SymbolsRef input, StateRef next, SymbolsRef output,
             Moves moves)
      : curr(curr), input(input), next(next), output(output),
        moves(std::move(moves)) {}

  using StateInput = std::pair<State, Symbols>;
  using StateOutput = std::tuple<State, Symbols, Moves>;

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

  auto convertTransitions(const TuringState &state) const
      -> std::set<Transition>;

  auto operator<=>(const Transition &other) const {
    return std::tie(curr, input, next, output, moves) <=>
           std::tie(other.curr, other.input, other.next, other.output,
                    other.moves);
  }
};

struct Transitions {
public:
  using TransitionMap =
      std::map<Transition::StateInput, Transition::StateOutput>;
  using iterator = TransitionMap::iterator;
  using const_iterator = TransitionMap::const_iterator;
  using value_type = TransitionMap::value_type;

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

  auto get(const Transition::StateInput &stateInput) const
      -> Transition::StateOutput {
    return transitions.find(stateInput)->second;
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
                                         "  symbols: [{}]\n"
                                         "  states: [{}]\n"
                                         "  tapeSymbols: [{}]\n"
                                         "  initialState: {}\n"
                                         "  blankSymbol: {}\n"
                                         "  finalStates: [{}]\n"
                                         "  tapeCount: {}\n"
                                         "  transitions: [\n{}\n"
                                         "  ]\n"
                                         "  totalTransitions: {}\n"
                                         "}";

  auto toString() -> std::string {
    return utils::format(FormatTemplate,           //
                         utils::join(symbols),     //
                         utils::join(states),      //
                         utils::join(tapeSymbols), //
                         initialState,             //
                         blankSymbol,              //
                         utils::join(finalStates), //
                         tapeCount,                //
                         transitions,              //
                         transitions.size());
  }
};

inline auto Transition::convertTransitions(const TuringState &state) const
    -> std::set<Transition> {
  auto queue = std::queue<Transition>();
  auto result = std::set<Transition>();
  queue.push(*this);
  while (!queue.empty()) {
    auto front = queue.front();
    queue.pop();
    bool flag = true;
    for (auto i = 0; i < front.input.size(); i++) {
      auto s1 = front.input[i];
      auto s2 = front.output[i];
      if (s1 == '*' || s2 == '*') {
        flag = false;
        auto nt = front;
        if (s1 == '*' && s2 == '*') {
          for (auto s : state.symbols) {
            if (s != state.blankSymbol) {
              nt.input[i] = s;
              nt.output[i] = s;
              queue.push(nt);
            }
          }
        } else if (s1 == '*') {
          for (auto s : state.symbols) {
            if (s != state.blankSymbol) {
              nt.input[i] = s;
              queue.push(nt);
            }
          }
        } else {
          for (auto s : state.symbols) {
            if (s != state.blankSymbol) {
              nt.output[i] = s;
              queue.push(nt);
            }
          }
        }
      }
    }
    if (flag) {
      result.insert(front);
    }
  }
  return result;
}
} // namespace turing::machine
