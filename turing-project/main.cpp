#include <Logger.h>
#include <Parser.h>

using turing::parser::Parser;
using turing::utils::Error;
using turing::utils::Logger;

auto main(int argc, char **argv) -> int {
  auto parser = Parser::fromArgs(argc, argv);
  const auto &logger = Logger::instance();

  auto simulator = parser.parse().onError([&logger](const Error &error) {
    logger.error(error.message());
    std::exit(error.value());
  });

  simulator.run().onError([&logger](const Error &error) {
    // logger.error(error.message()); // not required
    std::exit(error.value());
  });

  return 0;
}
