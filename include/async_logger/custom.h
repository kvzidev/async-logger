#pragma once

#include <async_logger/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#include <chrono>
#include <string>
#include <string_view>
#include <thread>

namespace async_logger {

class Logger;

/**
 * @brief Represents a single log message.
 */
struct LogMessage {
  std::chrono::system_clock::time_point m_timestamp;
  Logger_Level m_level;
  std::thread::id m_thread_id;
  std::string m_content;

  LogMessage(Logger_Level level, std::string content)
      : m_timestamp(std::chrono::system_clock::now()),
        m_level(level),
        m_thread_id(std::this_thread::get_id()),
        m_content(std::move(content)) {}
};

/**
 * @brief Abstract base class for all log formatters.
 */
class LogFormatter {
 public:
  virtual ~LogFormatter() = default;
  virtual std::string format(const LogMessage &msg) = 0;
};

/**
 * @brief Abstract base class for all log sinks.
 */
class LogSink {
 public:
  virtual ~LogSink() = default;
  virtual void write(const LogMessage &msg, std::string_view formatted) = 0;
  virtual void flush() = 0;
  /**
   * @brief Hook called when the sink is added to a logger.
   */
  virtual void set_status(Logger *logger, Logger_Status status) = 0;
};

}  // namespace async_logger
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief C callback for custom formatting.
 */
typedef void (*Logger_Formatter_Callback)(void *ctx, Logger_Level level,
                                          const char *content,
                                          uint64_t timestamp_ms,
                                          uint64_t thread_id, char *out_buf,
                                          size_t out_size);

/**
 * @brief C callback for custom sinks.
 */
typedef void (*Logger_Sink_Callback)(void *ctx, Logger_Level level,
                                     const char *message);

#ifdef __cplusplus
}
#endif
