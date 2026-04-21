#define ASYNC_LOGGER_MIN_LEVEL ASYNC_LOGGER_LEVEL_WARN
#include <iostream>
#include <logger>
#include <random>
#include <string>
#include <thread>
#include <vector>

using n_it = const size_t;

/**
 * @brief Worker function to simulate a concurrent data stream logging activity.
 * @param logger Reference to the shared logger instance.
 * @param stream_id Identifier for the stream/thread.
 * @param iterations Number of log messages to generate.
 */
void LoggingWorker(async_logger::Logger &logger, const int stream_id,
                   n_it iterations) {
  std::random_device rand_dev;
  std::mt19937 gen(rand_dev());
  std::uniform_int_distribution<> level_dist(0, 5);
  std::uniform_int_distribution<> sleep_dist(10, 50);

  for (size_t i = 0; i < iterations; ++i) {
    auto level = static_cast<async_logger::Level>(level_dist(gen));
    std::string message = "Thread " + std::to_string(stream_id) +
                          " - iteration " + std::to_string(i);

    switch (level) {
      case async_logger::Level::Logger_Level_Trace:
        logger.Trace(message.c_str());
        break;
      case async_logger::Level::Logger_Level_Debug:
        logger.Debug(message.c_str());
        break;
      case async_logger::Level::Logger_Level_Info:
        logger.Info(message.c_str());
        break;
      case async_logger::Level::Logger_Level_Warn:
        logger.Warn(message.c_str());
        break;
      case async_logger::Level::Logger_Level_Error:
        logger.Error(message.c_str());
        break;
      case async_logger::Level::Logger_Level_Fatal:
        logger.Fatal(message.c_str());
        break;
      default: break;
    }

    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(gen)));
  }
}

int main() {
  const int num_streams = 5;
  const int logs_per_stream = 50;

  async_logger::Logger logger = async_logger::Logger::New(
      "output-concurrent.log", Logger_Out_File | Logger_Out_Console);
  if (!logger) {
    std::cerr << "Failed to initialize logger\n";
    return 1;
  }

  logger.Info("Concurrent logging session starting");

  std::vector<std::jthread> streams;
  streams.reserve(num_streams);
  for (int i = 0; i < num_streams; ++i) {
    streams.emplace_back(LoggingWorker, std::ref(logger), i, logs_per_stream);
  }

  for (auto &thread : streams) {
    if (thread.joinable()) { thread.join(); }
  }

  logger.Info("All streams completed logging.");

  return 0;
}
