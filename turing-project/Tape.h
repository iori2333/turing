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
  Symbol blank;

  static constexpr auto FormatTemplate = "Index{} : {}\n"
                                         "Tape{}  : {}\n"
                                         "Head{}  : {}";

public:
  Tape(Size index, const TuringState &state)
      : index(index), blank(state.blankSymbol), _start(0), _head(0),
        tape(1, state.blankSymbol) {}

  Tape(Size index, const TuringState &state, SymbolsRef tape)
      : index(index), blank(state.blankSymbol), _start(0), _head(0),
        tape(tape) {}

  auto offset(Position pos) const -> Position { return pos - start(); }
  auto head() const -> Position { return _head; }
  auto start() const -> Position { return _start; }
  auto stop() const -> Position { return _start + tape.size(); }

  auto at(Position pos) const -> Symbol {
    if (pos < start() || pos >= stop()) {
      return blank;
    }
    return tape[offset(pos)];
  }

  auto at(Position pos) -> Symbol & {
    if (pos < start()) {
      tape.insert(tape.begin(), start() - pos, blank);
      _start = pos;
    } else if (pos >= stop()) {
      tape.insert(tape.end(), pos - stop() + 1, blank);
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
    auto startRealPos = static_cast<int>(tape.find_first_not_of(blank));
    auto stopRealPos = static_cast<int>(tape.find_last_not_of(blank));

    if (startRealPos == -1) {
      auto line = std::vector<std::string>{
          utils::toString(head()),
          utils::toString(blank),
          "^",
      };
      utils::alignRight(line);

      return utils::format(FormatTemplate, //
                           index, line[0], //
                           index, line[1], //
                           index, line[2]);
    }

    auto indexString = std::vector<std::string>{};
    auto tapeString = std::vector<std::string>{};
    auto headString = std::vector<std::string>{};

    startRealPos = std::min(startRealPos, offset(head()));
    stopRealPos = std::max(stopRealPos, offset(head()));

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

  auto setIndex(Size newIndex) -> void { index = newIndex; }

  auto result() const -> std::string {
    auto startRealPos = static_cast<int>(tape.find_first_not_of(blank));
    auto stopRealPos = static_cast<int>(tape.find_last_not_of(blank));

    if (startRealPos == -1) {
      return "";
    }

    return tape.substr(startRealPos, stopRealPos - startRealPos + 1);
  }
};

struct Tapes {
public:
  using Container = std::vector<Tape>;
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;
  using value_type = Container::value_type;

private:
  Container tapes;

public:
  explicit Tapes(Container tapes) : tapes(std::move(tapes)) {
    for (auto i = 0; i < tapes.size(); i++) {
      tapes[i].setIndex(i);
    }
  }

  Tapes(const TuringState &state, SymbolsRef first) {
    tapes.reserve(state.tapeCount);
    for (auto i = 0; i < state.tapeCount; i++) {
      if (i == 0) {
        tapes.emplace_back(i, state, first);
      } else {
        tapes.emplace_back(i, state);
      }
    }
  }

  explicit Tapes(const TuringState &state) {
    tapes.reserve(state.tapeCount);
    for (auto i = 0; i < state.tapeCount; i++) {
      tapes.emplace_back(i, state);
    }
  }

  auto read() const -> Symbols {
    auto symbols = Symbols{};
    symbols.reserve(tapes.size());
    std::transform(tapes.begin(), tapes.end(), std::back_inserter(symbols),
                   [](const auto &tape) { return tape.read(); });
    return symbols;
  }

  auto write(SymbolsRef symbols, MovesRef moves) -> std::vector<Position> {
    auto heads = std::vector<Position>{};
    heads.reserve(tapes.size());
    for (auto i = 0; i < tapes.size(); i++) {
      heads.emplace_back(tapes[i].write(symbols[i], moves[i]));
    }
    return heads;
  }

  auto operator[](Size index) -> Tape & { return tapes[index]; }
  auto operator[](Size index) const -> const Tape & { return tapes[index]; }

  auto begin() -> iterator { return tapes.begin(); }
  auto begin() const -> const_iterator { return tapes.begin(); }

  auto end() -> iterator { return tapes.end(); }
  auto end() const -> const_iterator { return tapes.end(); }

  auto toString() const -> std::string { return utils::join(*this, '\n'); }
  auto result() const -> std::string { return tapes[0].result(); }
};
} // namespace turing::machine
