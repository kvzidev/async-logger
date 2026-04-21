#pragma once

#include <async_logger/custom.h>

#include <iostream>
#include <mutex>

namespace async_logger {

/**
 * @brief Sink that writes log messages to the standard output.
 */
class ConsoleSink: public LogSink {
 public:
  void write(const LogMessage &, std::string_view formatted) override {
    std::cout << formatted << std::flush;
  }

  void flush() override { std::cout.flush(); }

  void set_status(Logger *, Logger_Status) override {}
};

}  // namespace async_logger
