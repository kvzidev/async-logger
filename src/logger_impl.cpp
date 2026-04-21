#include "logger_impl.hpp"

#include <format>
#include <mutex>

#include "console_sink.hpp"
#include "file_sink.hpp"
#include "log_formatter.hpp"

namespace async_logger {

LoggerImpl::LoggerImpl(const char *filename, int flags)
    : m_status(Logger_OK), m_formatter(std::make_unique<DefaultFormatter>()) {
  if ((flags & Console) != 0) { add_sink(std::make_unique<ConsoleSink>()); }
  if (((flags & File) != 0) && filename != nullptr) {
    add_sink(std::make_unique<FileSink>(filename));
  }
  init_worker();
}

void LoggerImpl::log(Logger_Level level, std::string content) {
  m_queue.push(LogMessage(level, std::move(content)));
}

void LoggerImpl::flush() {
  m_queue.wait_until_empty();
  std::scoped_lock lock(m_sinks_mutex);
  for (auto &sink : m_sinks) { sink->flush(); }
}

Logger_Status LoggerImpl::get_status() const { return m_status.load(); }

void LoggerImpl::set_status(Logger_Status status) { m_status.store(status); }

void LoggerImpl::set_max_file_size(size_t bytes) {
  std::scoped_lock lock(m_sinks_mutex);
  for (auto &sink : m_sinks) {
    if (auto *file_sink = dynamic_cast<FileSink *>(sink.get())) {
      file_sink->set_max_file_size(bytes);
    }
  }
}

void LoggerImpl::set_max_files(int count) {
  std::scoped_lock lock(m_sinks_mutex);
  for (auto &sink : m_sinks) {
    if (auto *file_sink = dynamic_cast<FileSink *>(sink.get())) {
      file_sink->set_max_files(count);
    }
  }
}

void LoggerImpl::add_sink(std::unique_ptr<LogSink> sink) {
  std::scoped_lock lock(m_sinks_mutex);
  m_sinks.push_back(std::move(sink));
}

void LoggerImpl::report_sink_status(Logger *owner) {
  std::scoped_lock lock(m_sinks_mutex);
  for (auto &sink : m_sinks) { sink->set_status(owner, Logger_OK); }
}

void LoggerImpl::set_formatter(std::unique_ptr<LogFormatter> formatter) {
  std::scoped_lock lock(m_sinks_mutex);
  m_formatter = std::move(formatter);
}

void LoggerImpl::init_worker() {
  m_worker = std::jthread(
      [this](const std::stop_token &stop) { this->worker_loop(stop); });
}

void LoggerImpl::worker_loop(const std::stop_token &stop) {
  while (!stop.stop_requested() || !m_queue.empty()) {
    auto msg = m_queue.pop(stop);
    if (msg) {
      std::string formatted;
      {
        std::scoped_lock lock(m_sinks_mutex);
        formatted = m_formatter->format(*msg);
      }

      std::scoped_lock lock(m_sinks_mutex);
      for (auto &sink : m_sinks) {
        try {
          sink->write(*msg, formatted);
        } catch (...) { m_status.store(Logger_Error_Write); }
      }
    }
  }
}

}  // namespace async_logger
