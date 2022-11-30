#include <Logger.h>
#include <Parser.h>

using turing::parser::Parser;
using turing::utils::Error;
using turing::utils::Logger;

[[noreturn]] inline auto errorHandler(const Error &error) {
  Logger::instance().info(error.message());
  std::exit(1);
}

auto main(int argc, char **argv) -> int {
  auto parser = Parser::fromArgs(argc, argv);
  auto simulator = parser.parse().onError(errorHandler);
  simulator.run().onError(errorHandler);
  return 0;
}
