#include "file_sink.hpp"

#include <logger>

namespace async_logger {

void FileSink::set_status(Logger *logger, Logger_Status status) {
  if (logger != nullptr) {
    if (status != Logger_Status::Logger_OK) { logger->SetStatus(status); }
    else if (m_initial_status != Logger_Status::Logger_OK) {
      logger->SetStatus(m_initial_status);
    }
  }
}

}  // namespace async_logger
