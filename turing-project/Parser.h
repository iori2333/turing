#pragma once
#include "StringUtils.h"
#include <fstream>
#include <regex>

#include <Errors.h>
#include <Logger.h>
#include <Machine.h>
#include <Simulator.h>

namespace turing::parser {

namespace constants {

using namespace std::literals::string_view_literals;

constexpr auto Usage = "usage: turing [-v|--verbose] [-h|--help] <tm> <input>";
constexpr auto EmptyString = ""sv;

constexpr auto StatesFlag = "#Q";
constexpr auto SymbolsFlags = "#S";
constexpr auto TapeSymbolsFlags = "#G";
constexpr auto InitialStateFlags = "#q0";
constexpr auto BlankSymbolFlag = "#B";
constexpr auto FinalStatesFlag = "#F";
constexpr auto TapeCountFlag = "#N";
constexpr auto CommentFlag = ';';

constexpr auto InvalidSymbols = " ,;{}*_"sv;
constexpr auto InvalidTapeSymbols = " ,;{}*"sv;

} // namespace constants

using machine::Move;
using machine::Moves;
using machine::Transition;
using machine::TuringState;
using simulator::Simulator;
using utils::Error;
using utils::Logger;
using utils::Result;
using utils::TuringError;

struct Parser {
private:
  std::ifstream fs;
  TuringState turingState;

  const Logger &logger;
  std::string_view input;

  static auto trimComments(std::string_view line) -> std::string_view {
    auto commentPos = line.find(constants::CommentFlag);
    if (commentPos != std::string_view::npos) {
      return line.substr(0, commentPos);
    }
    return line;
  }

public:
  explicit Parser(std::string_view filename, std::string_view input)
      : input(input), fs(filename.data()), logger(Logger::instance()) {
    if (!fs.is_open()) {
      logger.error("failed to open file: {}", filename);
      std::exit(1);
    }
  }

  static Parser fromArgs(int argc, char **argv) {
    auto &logger = Logger::instance();
    if (argc < 2) {
      logger.info(constants::Usage);
      std::exit(1);
    }

    auto args = std::vector<std::string_view>(argv + 1, argv + argc);
    auto filename = constants::EmptyString;
    auto input = constants::EmptyString;
    auto doHelp = false;
    auto doVerbose = false;
    for (auto arg : args) {
      if (!doVerbose && (arg == "-v" || arg == "--verbose")) {
        doVerbose = true;
      } else if (!doHelp && (arg == "-h" || arg == "--help")) {
        doHelp = true;
      } else if (filename.empty()) {
        filename = arg;
      } else if (input.empty()) {
        input = arg;
      }
    }

    logger.setVerbose(doVerbose);
    if (doHelp) {
      logger.info(constants::Usage);
      std::exit(0);
    }

    if (!filename.ends_with(".tm")) {
      logger.error("No input file specified");
      std::exit(1);
    }

    return Parser(filename, input);
  }

  auto parse() -> Result<Simulator> {
    while (!fs.eof()) {
      auto rLine = std::string{};
      std::getline(fs, rLine, '\n');
      auto line = trimComments(rLine);
      line = utils::trim(line);
      if (line.empty()) {
        continue;
      }

      Error e;
      if (line.starts_with(constants::StatesFlag)) {
        e = parseStates(line);
      } else if (line.starts_with(constants::SymbolsFlags)) {
        e = parseSymbols(line);
      } else if (line.starts_with(constants::TapeSymbolsFlags)) {
        e = parseTapeSymbols(line);
      } else if (line.starts_with(constants::InitialStateFlags)) {
        e = parseInitialState(line);
      } else if (line.starts_with(constants::BlankSymbolFlag)) {
        e = parseBlankSymbol(line);
      } else if (line.starts_with(constants::FinalStatesFlag)) {
        e = parseFinalStates(line);
      } else if (line.starts_with(constants::TapeCountFlag)) {
        e = parseTapeCount(line);
      } else {
        e = parseTransitions(line);
      }
      if (e != TuringError::Ok) {
        return e;
      }
    }

    return Simulator::of(std::move(turingState), input);
  }

  auto parseStates(std::string_view line) -> Error {
    static auto statesReg = std::regex{R"(#Q\s*=\s*\{([a-zA-Z0-9_, ]+)\})"};
    auto match = std::cmatch{};
    if (!std::regex_match(line.begin(), line.end(), match, statesReg)) {
      return TuringError::ParserInvalidStates;
    }
    auto stateString = utils::replace(match[1].str(), " ", "");
    auto lineStates = utils::split(stateString, ',');
    for (auto state : lineStates) {
      if (state.empty()) {
        return TuringError::ParserInvalidStates;
      }
      turingState.states.emplace(state);
    }
    return TuringError::Ok;
  }

  auto parseSymbols(std::string_view line) -> Error {
    static auto symbolsReg = std::regex{R"(#S\s*=\s*\{(.*)\})"};
    auto match = std::cmatch{};
    if (!std::regex_match(line.begin(), line.end(), match, symbolsReg)) {
      return TuringError::ParserInvalidSymbols;
    }
    auto symbolString = utils::replace(match[1].str(), " ", "");
    if (symbolString.empty()) {
      return TuringError::Ok;
    }
    auto lineSymbols = utils::split(symbolString, ',');
    for (auto symbol : lineSymbols) {
      if (symbol.size() != 1) {
        return TuringError::ParserInvalidSymbols;
      }
      auto sym = symbol[0];
      if (sym < 32 || sym > 126 ||
          constants::InvalidSymbols.find(sym) != std::string_view::npos) {
        return TuringError::ParserInvalidSymbols;
      }
      turingState.symbols.emplace(sym);
    }
    return TuringError::Ok;
  }

  auto parseTapeSymbols(std::string_view line) -> Error {
    static auto tapeSymbolsReg = std::regex{R"(#G\s*=\s*\{(.*)\})"};
    auto match = std::cmatch{};
    if (!std::regex_match(line.begin(), line.end(), match, tapeSymbolsReg)) {
      return TuringError::ParserInvalidTapeSymbols;
    }
    auto tapeSymbolString = utils::replace(match[1].str(), " ", "");
    auto lineTapeSymbols = utils::split(tapeSymbolString, ',');
    for (auto tapeSymbol : lineTapeSymbols) {
      if (tapeSymbol.size() != 1) {
        return TuringError::ParserInvalidTapeSymbols;
      }
      auto tapeSym = tapeSymbol[0];
      if (tapeSym < 32 || tapeSym > 126 ||
          constants::InvalidTapeSymbols.find(tapeSym) !=
              std::string_view::npos) {
        return TuringError::ParserInvalidTapeSymbols;
      }
      turingState.tapeSymbols.emplace(tapeSym);
    }
    return TuringError::Ok;
  }

  auto parseInitialState(std::string_view line) -> Error {
    static auto initialStateReg = std::regex{R"(#q0\s*=\s*([a-zA-Z0-9_]+))"};
    if (!turingState.initialState.empty()) {
      return TuringError::ParserDuplicateDefinition;
    }
    auto match = std::cmatch{};
    if (!std::regex_match(line.begin(), line.end(), match, initialStateReg)) {
      return TuringError::ParserInvalidInitialState;
    }
    turingState.initialState = match[1].str();
    return TuringError::Ok;
  }

  auto parseBlankSymbol(std::string_view line) -> Error {
    static auto blankSymbolReg = std::regex{R"(#B\s*=\s*([a-zA-Z0-9_]+))"};
    auto match = std::cmatch{};
    if (!std::regex_match(line.begin(), line.end(), match, blankSymbolReg)) {
      return TuringError::ParserInvalidBlankSymbol;
    }
    auto blankSymbol = match[1].str();
    if (blankSymbol.size() != 1) {
      return TuringError::ParserInvalidBlankSymbol;
    }
    auto sym = blankSymbol[0];
    if (sym != '_') {
      return TuringError::ParserInvalidBlankSymbol;
    }
    turingState.blankSymbol = sym;
    return TuringError::Ok;
  }

  auto parseFinalStates(std::string_view line) -> Error {
    static auto finalStatesReg =
        std::regex{R"(#F\s*=\s*\{([a-zA-Z0-9_, ]*)\})"};
    auto match = std::cmatch{};
    if (!std::regex_match(line.begin(), line.end(), match, finalStatesReg)) {
      return TuringError::ParserInvalidFinalStates;
    }
    auto finalStateString = utils::replace(match[1].str(), " ", "");
    if (finalStateString.empty()) {
      return TuringError::Ok;
    }
    auto lineFinalStates = utils::split(finalStateString, ',');
    for (auto finalState : lineFinalStates) {
      if (finalState.empty()) {
        return TuringError::ParserInvalidFinalStates;
      }
      turingState.finalStates.emplace(finalState);
    }
    return TuringError::Ok;
  }

  auto parseTapeCount(std::string_view line) -> Error {
    static auto tapeCountReg = std::regex{R"(#N\s*=\s*(\d+))"};
    auto match = std::cmatch{};
    if (!std::regex_match(line.begin(), line.end(), match, tapeCountReg)) {
      return TuringError::ParserInvalidTapeCount;
    }
    auto tapeCount = std::stoi(match[1].str());
    if (tapeCount < 1) {
      return TuringError::ParserInvalidTapeCount;
    }
    turingState.tapeCount = tapeCount;
    return TuringError::Ok;
  }

  auto parseTransitions(std::string_view line) -> Error {
    auto symbols = utils::split(line);
    utils::omitEmpty(symbols);
    if (symbols.size() != 5) {
      return TuringError::ParserInvalidTransition;
    }

    auto state = symbols[0];
    auto symbol = symbols[1];
    auto nextSymbol = symbols[2];
    auto direction = symbols[3];

    if (symbol.size() != turingState.tapeCount ||
        nextSymbol.size() != turingState.tapeCount ||
        direction.size() != turingState.tapeCount) {
      return TuringError::ParserInvalidTransition;
    }

    auto moves = Moves{};
    moves.reserve(turingState.tapeCount);
    for (auto ch : direction) {
      switch (ch) {
      case 'l':
        moves.emplace_back(Move::Left);
        break;
      case 'r':
        moves.emplace_back(Move::Right);
        break;
      case '*':
        moves.emplace_back(Move::Stay);
        break;
      default:
        return TuringError::ParserInvalidTransition;
      }
    }
    auto nextState = symbols[4];

    auto transition = Transition(state, symbol, nextState, nextSymbol, moves);
    if (!transition.isValid(turingState)) {
      return TuringError::ParserInvalidTransition;
    }

    if (transition.isStarTransition()) {
      for (auto &&convertedTransition :
           transition.convertTransitions(turingState)) {
        turingState.transitions.insert(std::move(convertedTransition));
      }
    } else {
      turingState.transitions.insert(std::move(transition));
    }
    return TuringError::Ok;
  }
};
} // namespace turing::parser
