#include <format>
#include <iostream>
#include <logger>
#include <memory>

/**
 * @brief A custom formatter that outputs logs in a simplified CSV-like format.
 */
class CsvFormatter: public async_logger::LogFormatter {
 public:
  std::string format(const async_logger::LogMessage &msg) override {
    std::string_view level_str = "UNKNOWN";
    switch (msg.m_level) {
      case async_logger::Level::Logger_Level_Trace: level_str = "TRACE"; break;
      case async_logger::Level::Logger_Level_Debug: level_str = "DEBUG"; break;
      case async_logger::Level::Logger_Level_Info: level_str = "INFO"; break;
      case async_logger::Level::Logger_Level_Warn: level_str = "WARN"; break;
      case async_logger::Level::Logger_Level_Error: level_str = "ERROR"; break;
      case async_logger::Level::Logger_Level_Fatal: level_str = "FATAL"; break;
      default: break;
    }

    // Format: Timestamp,Level,Content
    return std::format(
        "{:%Y%m%d-%H%M%S},{},\"{}\"\n",
        std::chrono::floor<std::chrono::seconds>(msg.m_timestamp), level_str,
        msg.m_content);
  }
};

int main() {
  async_logger::Logger logger =
      async_logger::Logger::New(nullptr, Logger_Out_Console);

  std::cout << "Standard Formatting\n";
  logger.Info("This uses the default formatter.");
  logger.Flush();

  std::cout << "\nSwitch to CSV Formatting\n";
  logger.SetFormatter(std::make_unique<CsvFormatter>());

  logger.Info("This now uses the CSV formatter.");
  logger.Warn("Multiple sinks would also use this format.");
  logger.Error("Custom formatters are powerful!");

  logger.Flush();
  return 0;
}
