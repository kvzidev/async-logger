#pragma once

#include <async_logger/types.h>
#include <logger.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "synchronized_queue.hpp"

namespace async_logger {

class LogFormatter;
class LogSink;

/**
 * @brief Implementation details for the Logger class.
 */
class LoggerImpl {
 public:
  enum SinkFlags { Console = Logger_Out_Console, File = Logger_Out_File };

  LoggerImpl(const char *filename, int flags);
  ~LoggerImpl() = default;

  void log(Logger_Level level, std::string content);
  void flush();

  Logger_Status get_status() const;
  void set_status(Logger_Status status);

  void set_max_file_size(size_t bytes);
  void set_max_files(int count);

  void add_sink(std::unique_ptr<LogSink> sink);
  void report_sink_status(Logger *owner);

  void set_formatter(std::unique_ptr<LogFormatter> formatter);

 private:
  void init_worker();
  void worker_loop(const std::stop_token &stop);

 private:
  SynchronizedQueue m_queue;
  std::vector<std::unique_ptr<LogSink>> m_sinks;
  mutable std::recursive_mutex m_sinks_mutex;
  std::atomic<Logger_Status> m_status;
  std::unique_ptr<LogFormatter> m_formatter;
  std::jthread m_worker;
};

}  // namespace async_logger
