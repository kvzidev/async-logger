#include <array>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <logger>
#include <print>
#include <sstream>
#include <string>

#include "async_logger/custom.h"
#include "logger_impl.hpp"

/**
 * @brief Internal wrapper for the C++ implementation.
 * Defined globally as it is the basis for the C Logger_t handle.
 */
struct LoggerHandle {
  async_logger::LoggerImpl impl;
  LoggerHandle(const char *filename, int flags): impl(filename, flags) {}
};

namespace async_logger {

// MARK: C++ API Impl

Logger Logger::New(const char *filename, int flags) {
  auto *handle =
      reinterpret_cast<::LoggerHandle *>(Logger_New(filename, flags));
  if (handle == nullptr) { return {}; }

  Logger logger(handle);
  handle->impl.report_sink_status(&logger);
  return logger;
}

Logger::~Logger() { Free(); }

void Logger::Log(Level level, const char *message) {
  Logger_Log(reinterpret_cast<Logger_t *>(m_handle), level, message);
}

void Logger::AddSink(std::unique_ptr<LogSink> sink) {
  if (m_handle != nullptr && sink != nullptr) {
    sink->set_status(this, Logger_OK);
    m_handle->impl.add_sink(std::move(sink));
  }
}

void Logger::SetFormatter(std::unique_ptr<LogFormatter> formatter) {
  if (m_handle != nullptr && formatter != nullptr) {
    m_handle->impl.set_formatter(std::move(formatter));
  }
}

void Logger::SetStatus(Status status) {
  if (m_handle != nullptr) { m_handle->impl.set_status(status); }
}

Status Logger::GetStatus() const {
  if (m_handle == nullptr) { return Logger_Error_Uninitialized; }
  return m_handle->impl.get_status();
}

void Logger::SetMaxFileSize(std::size_t bytes) {
  if (m_handle != nullptr) { m_handle->impl.set_max_file_size(bytes); }
}

void Logger::SetMaxFiles(int count) {
  if (m_handle != nullptr) { m_handle->impl.set_max_files(count); }
}

void Logger::Flush() { Logger_Flush(reinterpret_cast<Logger_t *>(m_handle)); }

void Logger::Free() {
  if (m_handle != nullptr) {
    Logger_Free(reinterpret_cast<Logger_t *>(m_handle));
    m_handle = nullptr;
  }
}

}  // namespace async_logger

// MARK: C API Impl

extern "C" {

Logger_t *Logger_New(const char *filename, int flags) {
  try {
    auto *handle = new LoggerHandle(filename, flags);

    // Pass to C++ wrapper temporarily to report sink status
    async_logger::Logger wrapper;
    wrapper.SetHandleForCAPIOnly(handle);
    handle->impl.report_sink_status(&wrapper);
    wrapper.SetHandleForCAPIOnly(nullptr);

    return reinterpret_cast<Logger_t *>(handle);
  } catch (const std::exception &e) {
    std::cerr << "Failed to create logger: " << e.what() << '\n';
    return nullptr;
  }
}

void Logger_AddSink(Logger_t *logger, Logger_Sink_Callback callback,
                    void *ctx) {
  if (logger == nullptr || callback == nullptr) { return; }
  auto *handle = reinterpret_cast<LoggerHandle *>(logger);

  class CSinkAdapter: public async_logger::LogSink {
   public:
    CSinkAdapter(Logger_Sink_Callback callback, void *context)
        : m_callback(callback), m_ctx(context) {}

    void write(const async_logger::LogMessage &msg,
               std::string_view formatted) override {
      m_callback(m_ctx, msg.m_level, std::string(formatted).c_str());
    }
    void flush() override {}
    void set_status(async_logger::Logger * /*unused*/,
                    Logger_Status /*unused*/) override {}

   private:
    Logger_Sink_Callback m_callback;
    void *m_ctx;
  };

  auto sink = std::make_unique<CSinkAdapter>(callback, ctx);

  async_logger::Logger wrapper;
  wrapper.SetHandleForCAPIOnly(handle);
  sink->set_status(&wrapper, Logger_OK);
  wrapper.SetHandleForCAPIOnly(nullptr);

  handle->impl.add_sink(std::move(sink));
}

class CFormatterAdapter: public async_logger::LogFormatter {
 public:
  CFormatterAdapter(Logger_Formatter_Callback callback, void *ctx)
      : m_callback(callback), m_ctx(ctx) {}

  std::string format(const async_logger::LogMessage &msg) override {
    if (m_callback == nullptr) { return ""; }

    auto ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                     msg.m_timestamp.time_since_epoch())
                     .count();

    std::array<char, 4096> buffer;
    uint64_t tid = 0;
    std::stringstream sstream;
    sstream << msg.m_thread_id;
    try {
      tid = std::stoull(sstream.str());
    } catch (...) {
      std::println(stderr, "Failed to convert thread ID to uint64_t");
    }

    m_callback(m_ctx, msg.m_level, msg.m_content.c_str(), ts_ms, tid,
               buffer.data(), buffer.size());
    return std::string {buffer.data()};
  }

 private:
  Logger_Formatter_Callback m_callback;
  void *m_ctx;
};

void Logger_SetFormatter(Logger_t *logger, Logger_Formatter_Callback callback,
                         void *ctx) {
  if (logger == nullptr || callback == nullptr) { return; }
  auto *handle = reinterpret_cast<LoggerHandle *>(logger);
  handle->impl.set_formatter(
      std::make_unique<CFormatterAdapter>(callback, ctx));
}

void Logger_Free(Logger_t *logger) {
  if (logger != nullptr) { delete reinterpret_cast<LoggerHandle *>(logger); }
}

void Logger_Log(Logger_t *logger, Logger_Level level, const char *message) {
  if (logger == nullptr || message == nullptr) { return; }

  auto *handle = reinterpret_cast<LoggerHandle *>(logger);
  handle->impl.log(level, std::string(message));
}

void Logger_Flush(Logger_t *logger) {
  if (logger != nullptr) {
    auto *handle = reinterpret_cast<LoggerHandle *>(logger);
    handle->impl.flush();
  }
}

Logger_Status Logger_GetStatus(Logger_t *logger) {
  if (logger == nullptr) { return Logger_Error_Uninitialized; }

  auto *handle = reinterpret_cast<LoggerHandle *>(logger);
  return handle->impl.get_status();
}

void Logger_SetStatus(Logger_t *logger, Logger_Status status) {
  if (logger != nullptr) {
    auto *handle = reinterpret_cast<LoggerHandle *>(logger);
    handle->impl.set_status(status);
  }
}

void Logger_SetMaxFileSize(Logger_t *logger, size_t size_bytes) {
  if (logger != nullptr) {
    auto *handle = reinterpret_cast<LoggerHandle *>(logger);
    handle->impl.set_max_file_size(size_bytes);
  }
}

void Logger_SetMaxFiles(Logger_t *logger, int count) {
  if (logger != nullptr) {
    auto *handle = reinterpret_cast<LoggerHandle *>(logger);
    handle->impl.set_max_files(count);
  }
}

}  // extern "C"
