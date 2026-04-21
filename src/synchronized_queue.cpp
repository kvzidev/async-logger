#include "synchronized_queue.hpp"

namespace async_logger {

void SynchronizedQueue::push(LogMessage &&msg) {
  {
    std::scoped_lock lock(m_mutex);
    m_queue.push(std::move(msg));
  }
  m_cv.notify_one();
}

std::optional<LogMessage> SynchronizedQueue::pop(const std::stop_token &stop) {
  std::unique_lock lock(m_mutex);

  m_cv.wait(lock, stop, [this] { return !m_queue.empty(); });

  if (m_queue.empty()) { return std::nullopt; }

  LogMessage msg = std::move(m_queue.front());
  m_queue.pop();

  if (m_queue.empty()) { m_empty_cv.notify_all(); }

  return msg;
}

void SynchronizedQueue::wait_until_empty() const {
  std::unique_lock lock(m_mutex);
  m_empty_cv.wait(lock, [this] { return m_queue.empty(); });
}

bool SynchronizedQueue::empty() const {
  std::scoped_lock lock(m_mutex);
  return m_queue.empty();
}

}  // namespace async_logger
