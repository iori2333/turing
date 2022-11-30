#pragma once
#include <fstream>
#include <regex>
#include <string_view>
#include <unordered_set>

#include <Errors.h>
#include <Logger.h>
#include <Machine.h>
#include <Simulator.h>

namespace turing::parser {

namespace constants {

using namespace std::literals::string_view_literals;

constexpr auto StatesFlag = "#Q";
constexpr auto SymbolsFlags = "#S";
constexpr auto TapeSymbolsFlags = "#G";
constexpr auto InitialStateFlags = "#q0";
constexpr auto BlankSymbolFlag = "#B";
constexpr auto FinalStatesFlag = "#F";
constexpr auto TapeCountFlag = "#N";
constexpr auto CommentFlag = ';';

constexpr auto InvalidSymbols = " ,;{}*_"sv;

} // namespace constants

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

  struct Config {
    const Logger &logger;
    std::string_view filename;
    std::string_view input;
  } config;

  static auto trimComments(std::string_view line) -> std::string_view {
    auto commentPos = line.find(constants::CommentFlag);
    if (commentPos != std::string_view::npos) {
      return line.substr(0, commentPos);
    }
    return utils::trim(line);
  }

public:
  explicit Parser(Config config)
      : config(std::move(config)), fs(config.filename.data()) {
    if (!fs.is_open()) {
      config.logger.errorf("failed to open file: {}", config.filename);
      std::exit(1);
    }
  }

  static Parser fromArgs(int argc, char **argv) {
    if (argc < 2 || argc > 5) {
      Logger::instance().info(usage());
      std::exit(1);
    }

    auto args = std::vector<std::string_view>(argv + 1, argv + argc);
    auto logger = Logger::instance();
    auto config = Config{.logger = logger, .filename = ""};
    auto doHelp = false;
    for (auto &arg : args) {
      if (arg == "-v" || arg == "--verbose") {
        logger.setVerbose(true);
      } else if (arg == "-h" || arg == "--help") {
        doHelp = true;
      } else if (config.filename.empty()) {
        config.filename = arg;
      } else if (config.input.empty()) {
        config.input = arg;
      }
    }

    if (doHelp) {
      Logger::instance().info(usage());
      std::exit(0);
    }

    if (!config.filename.ends_with(".tm")) {
      Logger::instance().error("No input file specified");
      std::exit(1);
    }

    return Parser(std::move(config));
  }

  static auto usage() -> const char * {
    return "usage: turing [-v|--verbose] [-h|--help] <tm> <input>";
  }

  auto parse() -> Result<Simulator> {
    while (!fs.eof()) {
      auto rLine = std::string{};
      std::getline(fs, rLine, '\n');
      auto line = trimComments(rLine);
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
    return Simulator::of(std::move(turingState));
  }

  auto parseStates(std::string_view line) -> Error {
    static auto statesReg = std::regex{R"(#Q\s*=\s*\{([a-zA-Z0-9_, ]+)\})"};
    auto match = std::cmatch{};
    if (!std::regex_match(line.data(), match, statesReg)) {
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
    if (!std::regex_match(line.data(), match, symbolsReg)) {
      return TuringError::ParserInvalidSymbols;
    }
    auto symbolString = utils::replace(match[1].str(), " ", "");
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
      turingState.symbols.emplace(symbol[0]);
    }
    return TuringError::Ok;
  }

  auto parseTapeSymbols(std::string_view line) -> Error {
    return TuringError::Ok;
  }

  auto parseInitialState(std::string_view line) -> Error {
    return TuringError::Ok;
  }

  auto parseBlankSymbol(std::string_view line) -> Error {
    return TuringError::Ok;
  }

  auto parseFinalStates(std::string_view line) -> Error {
    return TuringError::Ok;
  }

  auto parseTapeCount(std::string_view line) -> Error {
    return TuringError::Ok;
  }

  auto parseTransitions(std::string_view line) -> Error {
    return TuringError::Ok;
  }
};
} // namespace turing::parser
