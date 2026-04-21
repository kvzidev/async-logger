#pragma once

#include <async_logger/custom.h>

#include <format>

namespace async_logger {

/**
 * @brief Default log formatter using std::format.
 */
class DefaultFormatter: public LogFormatter {
 public:
  std::string format(const LogMessage &msg) override {
    std::string_view level_str = "UNKNOWN";
    switch (msg.m_level) {
      case Logger_Level_Trace: level_str = "TRACE"; break;
      case Logger_Level_Debug: level_str = "DEBUG"; break;
      case Logger_Level_Info: level_str = "INFO "; break;
      case Logger_Level_Warn: level_str = "WARN "; break;
      case Logger_Level_Error: level_str = "ERROR"; break;
      case Logger_Level_Fatal: level_str = "FATAL"; break;
      default: break;
    }

    return std::format(
        "[{:%Y-%m-%d %H:%M:%S}] [{}] [Thread {}] {}\n",
        std::chrono::floor<std::chrono::seconds>(msg.m_timestamp), level_str,
        msg.m_thread_id, msg.m_content);
  }
};

}  // namespace async_logger
