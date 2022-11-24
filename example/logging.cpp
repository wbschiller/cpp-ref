
#include "spdlog/async.h"
#include "spdlog/fmt/chrono.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

/**
 * A simple example for using spdlog as a logger for a program
 */
auto main() -> int
{
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

  // auto spdlogger = spdlog::basic_logger_st("spdlog", "spdlog.txt", true);
  // default thread pool settings can be modified *before* creating the async
  // logger:  queue with 10,000 items and 1 backing thread
  spdlog::init_thread_pool(10000, 1);
  auto logger =
      spdlog::basic_logger_st<spdlog::async_factory>("examples", "log.txt");
  logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%^%L%$] %v");
  logger->set_level(spdlog::level::info);

  spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR,
               SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);

  spdlog::info("See {} for the remainder of the log entries", "log.txt");
  spdlog::set_default_logger(logger);

  spdlog::warn("Easy padding in numbers like {:08d}", 12);
  spdlog::critical(
      "Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
  auto now = std::chrono::system_clock::now();
  spdlog::info("Here's the current clock from std::chrono {}", now);

  spdlog::info("Support for floats {:03.2f}", 1.23456);
  spdlog::error("Positional args are {1} {0}..", "too", "supported");
  spdlog::info("{:>8} aligned, {:<8} aligned", "right", "left");

  // Runtime log levels
  spdlog::set_level(spdlog::level::info); // Set global log level to info
  spdlog::debug("This message should not be displayed!");
  spdlog::set_level(spdlog::level::trace); // Set specific logger's log level
  spdlog::debug("This message should be displayed..");

  // Customize msg format for all loggers - doesn't work with async logger
  // spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
  // spdlog::info("This an info message with custom format");
  // spdlog::set_pattern("%+"); // back to default format
  // spdlog::set_level(spdlog::level::info);
  // spdlog::info("This an info message back using default format");

  // Compile-time or runtime errors
  spdlog::info("Missing parameter: Compiler or runtimer error? {:>8} aligned, "
               "{:<8} aligned",
               "right");
  spdlog::info("Extra parameter: compiler or runtimer error? {:>8} aligned",
               "right", "left");

  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

  return 0;
}