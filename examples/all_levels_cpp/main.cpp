#define ASYNC_LOGGER_MIN_LEVEL ASYNC_LOGGER_LEVEL_INFO
#include <iostream>
#include <logger>

int main() {
  async_logger::Logger logger = async_logger::Logger::New(
      "all_levels.log", Logger_Out_Console | Logger_Out_File);

  if (!logger) {
    std::cerr << "Failed to initialize logger!" << '\n';
    return 1;
  }

  logger.Trace("This is a TRACE message - useful for deep debugging.");

  logger.Debug("This is a DEBUG message - shows internal state or logic flow.");

  logger.Info("This is an INFO message - normal application behavior.");

  logger.Warn(
      "This is a WARN message - something might be wrong, but we can "
      "continue.");

  logger.Error("This is an ERROR message - a specific operation failed.");

  logger.Fatal("This is a FATAL message - critical failure detected.");

  logger.Flush();

  return 0;
}
