#pragma once

#include <async_logger/custom.h>
#include <async_logger/types.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>

namespace async_logger {

/**
 * @brief Sink that writes log messages to a file with rotation support.
 */
class FileSink: public LogSink {
 public:
  explicit FileSink(const std::filesystem::path &filename)
      : m_base_path(filename), m_file(filename, std::ios::app) {
    if (m_file.is_open()) {
      try {
        if (std::filesystem::exists(filename)) {
          m_current_file_size = std::filesystem::file_size(filename);
        }
      } catch (...) {
        m_current_file_size = 0;
        m_initial_status = Logger_Status::Logger_Error_File_Open;
      }
    }
    else { m_initial_status = Logger_Status::Logger_Error_File_Open; }
  }

  void write(const LogMessage &, std::string_view formatted) override {
    if (!m_file.is_open()) { return; }

    size_t msg_size = formatted.size();
    size_t limit = m_max_file_size.load();

    if (limit > 0 && m_current_file_size + msg_size > limit) { rotate(); }

    m_file << formatted;
    m_current_file_size += msg_size;
  }

  void flush() override {
    if (m_file.is_open()) { m_file.flush(); }
  }

  void set_status(Logger *logger, Logger_Status status) override;

  void set_max_file_size(size_t bytes) { m_max_file_size.store(bytes); }
  void set_max_files(int count) { m_max_files.store(count); }

 private:
  void rotate() {
    m_file.close();

    int max_rotated = m_max_files.load();
    std::filesystem::path parent = m_base_path.parent_path();
    std::string stem = m_base_path.stem().string();
    std::string extension = m_base_path.extension().string();

    for (int i = max_rotated - 1; i >= 1; --i) {
      std::filesystem::path old_name =
          parent / (stem + "." + std::to_string(i) + extension);
      std::filesystem::path new_name =
          parent / (stem + "." + std::to_string(i + 1) + extension);

      std::error_code err_code;
      if (std::filesystem::exists(old_name, err_code)) {
        std::filesystem::rename(old_name, new_name, err_code);
      }
    }

    std::error_code err_code;
    if (std::filesystem::exists(m_base_path, err_code)) {
      std::filesystem::path rotated_name = parent / (stem + ".1" + extension);
      std::filesystem::rename(m_base_path, rotated_name, err_code);
    }

    m_file.open(m_base_path, std::ios::out);
    m_current_file_size = 0;
  }

 private:
  Logger_Status m_initial_status {Logger_Status::Logger_OK};
  std::filesystem::path m_base_path;
  std::ofstream m_file;
  std::atomic<size_t> m_max_file_size {10 * 1024 * 1024};
  std::atomic<int> m_max_files {5};
  size_t m_current_file_size {0};
};

}  // namespace async_logger
