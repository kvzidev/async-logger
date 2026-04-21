#pragma once

#include <async_logger/custom.h>

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>

namespace async_logger {

class SynchronizedQueue {
 public:
  void push(LogMessage &&msg);
  std::optional<LogMessage> pop(const std::stop_token &stop);
  bool empty() const;
  void wait_until_empty() const;

 private:
  std::queue<LogMessage> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable_any m_cv;
  mutable std::condition_variable m_empty_cv;
};

}  // namespace async_logger
