#pragma once
#include <errors.h>
#include <fstream>
#include <string_view>
#include <unordered_set>

#include <logger.h>

namespace turing::parser {

namespace constants {
using namespace std::string_view_literals;

constexpr auto StatesFlag = "#Q"sv;
constexpr auto SymbolsFlags = "#S"sv;
constexpr auto TapeSymbolsFlags = "#G"sv;
constexpr auto InitialStateFlags = "#q0";
constexpr auto BlankSymbolFlag = "#B"sv;
constexpr auto FinalStatesFlag = "#F"sv;
constexpr auto TapeCountFlag = "#N"sv;
constexpr auto CommentFlag = ';';

} // namespace constants

using turing::utils::Error;
using turing::utils::Logger;
using turing::utils::TuringError;

using SymbolSet = std::unordered_set<char>;

struct Parser {
private:
  std::ifstream fs;

  struct Config {
    const Logger &logger;
    std::string_view filename;
  } config;

  static auto trimComments(std::string_view line) -> std::string_view {
    auto commentPos = line.find(constants::CommentFlag);
    if (commentPos != std::string_view::npos) {
      return line.substr(0, commentPos);
    }
    return line;
  }

  static auto trim(std::string_view line) -> std::string_view {
    auto line2 = trimComments(line);
    auto start = line2.find_first_not_of(' ');
    auto end = line2.find_last_not_of(' ');
    if (start == std::string_view::npos || end == std::string_view::npos) {
      return "";
    }
    return line2.substr(start, end - start + 1);
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
      } else {
        config.filename = arg;
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

  auto parse() -> int {
    while (!fs.eof()) {
      auto rLine = std::string{};
      fs >> rLine;
      auto line = trim(rLine);
      if (line.empty()) {
        continue;
      }
      if (line.starts_with(constants::StatesFlag)) {
        parseStates(line);
      } else if (line.starts_with(constants::SymbolsFlags)) {
        parseSymbols(line);
      } else if (line.starts_with(constants::TapeSymbolsFlags)) {
        parseTapeSymbols(line);
      } else if (line.starts_with(constants::InitialStateFlags)) {
        parseInitialState(line);
      } else if (line.starts_with(constants::BlankSymbolFlag)) {
        parseBlankSymbol(line);
      } else if (line.starts_with(constants::FinalStatesFlag)) {
        parseFinalStates(line);
      } else if (line.starts_with(constants::TapeCountFlag)) {
        parseTapeCount(line);
      } else {
        parseTransitions(line);
      }
    }
  }

  auto parseStates(std::string_view line) -> Error { return TuringError::Ok; }

  auto parseSymbols(std::string_view line) -> Error {}

  auto parseTapeSymbols(std::string_view line) -> Error {}

  auto parseInitialState(std::string_view line) -> Error {}

  auto parseBlankSymbol(std::string_view line) -> Error {}

  auto parseFinalStates(std::string_view line) -> Error {}

  auto parseTapeCount(std::string_view line) -> Error {}

  auto parseTransitions(std::string_view line) -> Error {}
};
} // namespace turing::parser
