#pragma once
#include <Machine.h>
#include <StringUtils.h>

namespace turing::machine {
struct Tape {
private:
  Size index;
  Symbols tape;
  Position _start; // Offset of logical position and real position
  Position _head;  // Write _head
  const TuringState &state;

  static constexpr auto FormatTemplate = "Index{} : {}\n"
                                         "Tape{}  : {}\n"
                                         "Head{}  : {}";

public:
  Tape(Size index, const TuringState &state)
      : index(index), state(state), _start(0), _head(0),
        tape(1, state.blankSymbol) {}

  Tape(Size index, const TuringState &state, SymbolsRef tape)
      : index(index), state(state), _start(0), _head(0), tape(tape) {}

  auto offset(Position pos) const -> Position { return pos - start(); }
  auto head() const -> Position { return _head; }
  auto start() const -> Position { return _start; }
  auto stop() const -> Position { return _start + tape.size(); }

  auto at(Position pos) const -> Symbol {
    if (pos < start() || pos >= stop()) {
      return state.blankSymbol;
    }
    return tape[offset(pos)];
  }

  auto at(Position pos) -> Symbol & {
    if (pos < start()) {
      tape.insert(tape.begin(), start() - pos, state.blankSymbol);
      _start = pos;
    } else if (pos >= stop()) {
      tape.insert(tape.end(), pos - stop() + 1, state.blankSymbol);
    }
    return tape[offset(pos)];
  }

  auto operator[](Position pos) const -> Symbol { return at(pos); }
  auto operator[](Position pos) -> Symbol & { return at(pos); }

  auto write(Symbol symbol, Move move) -> Position {
    (*this)[head()] = symbol;
    _head += static_cast<Position>(move);
    return head();
  }

  auto read() const -> Symbol { return at(head()); }

  auto toString() const -> std::string {
    auto startRealPos = tape.find_first_not_of(state.blankSymbol);
    auto stopRealPos = tape.find_last_not_of(state.blankSymbol);

    auto indexString = std::vector<std::string>{};
    auto tapeString = std::vector<std::string>{};
    auto headString = std::vector<std::string>{};
    for (auto i = startRealPos; i <= stopRealPos; i++) {
      auto logicalPos = i + start();
      auto symbol = at(logicalPos);
      auto line = std::vector<std::string>{
          utils::toString(logicalPos),
          utils::toString(symbol),
          logicalPos == head() ? "^" : " ",
      };
      utils::alignRight(line);
      indexString.emplace_back(std::move(line[0]));
      tapeString.emplace_back(std::move(line[1]));
      headString.emplace_back(std::move(line[2]));
    }

    return utils::format(FormatTemplate,                  //
                         index, utils::join(indexString), //
                         index, utils::join(tapeString),  //
                         index, utils::join(headString));
  }

  auto turingState() const -> const TuringState & { return state; }

  auto setIndex(Size newIndex) -> void { index = newIndex; }
};

struct Tapes {
public:
  using Container = std::vector<Tape>;
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;
  using value_type = Container::value_type;

private:
  Container tapes;

  static constexpr auto SplitFlag =
      "\n---------------------------------------------\n";

public:
  explicit Tapes(Container tapes) : tapes(std::move(tapes)) {
    for (auto i = 0; i < tapes.size(); i++) {
      tapes[i].setIndex(i);
    }
  }

  Tapes(const TuringState &state, Symbols first) {
    tapes.reserve(state.tapeCount);
    for (auto i = 0; i < state.tapeCount; i++) {
      if (i == 0) {
        tapes.emplace_back(i, state, std::move(first));
      } else {
        tapes.emplace_back(i, state);
      }
    }
  }

  auto operator[](Size index) -> Tape & { return tapes[index]; }
  auto operator[](Size index) const -> const Tape & { return tapes[index]; }

  auto begin() { return tapes.begin(); }
  auto begin() const { return tapes.begin(); }

  auto end() { return tapes.end(); }
  auto end() const { return tapes.end(); }

  auto toString() -> std::string {
    auto ret = std::string{};
    for (const auto &tape : *this) {
      ret += tape.toString();
      ret += SplitFlag;
    }
    return ret;
  }
};
} // namespace turing::machine
