#pragma once

#include <async_logger/custom.h>
#include <async_logger/types.h>

#include <string>

namespace async_logger {

/**
 * @brief Sink that wraps a C callback for custom logging.
 */
class CustomSink: public LogSink {
 public:
  typedef void (*Callback)(void *ctx, Logger_Level level, const char *message);

  CustomSink(Callback cb, void *ctx): m_callback(cb), m_ctx(ctx) {}

  void write(const LogMessage &msg, std::string_view formatted) override {
    if (m_callback) {
      Logger_Level c_level;
      switch (msg.m_level) {
        case async_logger::Trace: c_level = Logger_Level_Trace; break;
        case async_logger::Debug: c_level = Logger_Level_Debug; break;
        case async_logger::Info: c_level = Logger_Level_Info; break;
        case async_logger::Warn: c_level = Logger_Level_Warn; break;
        case async_logger::Error: c_level = Logger_Level_Error; break;
        case async_logger::Fatal: c_level = Logger_Level_Fatal; break;
        default: c_level = Logger_Level_Info; break;
      }
      m_callback(m_ctx, c_level, std::string(formatted).c_str());
    }
  }

  void flush() override {}

  void set_status(Logger *, Status) override {}

 private:
  Callback m_callback;
  void *m_ctx;
};

}  // namespace async_logger
