#include <async_logger/custom.h>

#include <iostream>
#include <logger>
#include <mutex>
#include <vector>

/**
 * @brief A custom sink that stores log messages in memory (std::vector).
 */
class MemorySink: public async_logger::LogSink {
 public:
  void write(const async_logger::LogMessage &msg,
             std::string_view formatted) override {
    std::scoped_lock lock(m_mutex);
    m_logs.emplace_back(formatted);
  }

  void flush() override {}

  void set_status(async_logger::Logger * /*logger*/,
                  Logger_Status /*status*/) override {}

  /**
   * @brief Retrieves all captured logs and clears the storage.
   */
  std::vector<std::string> GetAndClear() {
    std::scoped_lock lock(m_mutex);
    std::vector<std::string> copy = std::move(m_logs);
    m_logs.clear();
    return copy;
  }

 private:
  std::mutex m_mutex;
  std::vector<std::string> m_logs;
};

int main() {
  auto memory_sink_ptr = std::make_unique<MemorySink>();
  // The Logger will take ownership of it when we call AddSink.
  // So we create a reference.
  MemorySink *sink_ref = memory_sink_ptr.get();

  async_logger::Logger logger =
      async_logger::Logger::New(nullptr, Logger_Out_Console);

  logger.AddSink(std::move(memory_sink_ptr));

  logger.Info("Starting application...");
  logger.Warn("This is a warning to memory.");
  logger.Error("System failure!");

  logger.Flush();

  std::cout << "\nRetrived from MemorySink\n";
  auto logs = sink_ref->GetAndClear();
  for (const auto &log : logs) { std::cout << "[Memory] " << log; }

  return 0;
}
