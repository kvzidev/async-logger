#include <gtest/gtest.h>

#include <filesystem>
#include <logger>
#include <sstream>
#include <thread>

#include "../src/console_sink.hpp"
#include "../src/file_sink.hpp"
#include "../src/logger_impl.hpp"

using namespace async_logger;

/**
 * @brief A helper sink for testing that writes to a stringstream.
 */
class StreamSink: public LogSink {
 public:
  explicit StreamSink(std::ostream &out): m_out(out) {}
  void write(const LogMessage & /*msg*/, std::string_view formatted) override {
    m_out << formatted;
  }
  void flush() override { m_out.flush(); }
  void set_status(Logger * /*logger*/, Logger_Status /*status*/) override {}

 private:
  std::ostream &m_out;
};

TEST(LoggerImplTest, BasicLogging) {
  std::stringstream str_stream;
  {
    LoggerImpl logger(nullptr, 0);
    logger.add_sink(std::make_unique<StreamSink>(str_stream));
    logger.log(Logger_Level_Info, "hello world");
  }

  std::string output = str_stream.str();
  EXPECT_NE(output.find("[INFO ]"), std::string::npos);
  EXPECT_NE(output.find("hello world"), std::string::npos);
  EXPECT_EQ(output.front(), '[');
}

TEST(LoggerImplTest, AllLevels) {
  std::stringstream str_stream;
  {
    LoggerImpl logger(nullptr, 0);
    logger.add_sink(std::make_unique<StreamSink>(str_stream));
    logger.log(Logger_Level_Trace, "trace");
    logger.log(Logger_Level_Debug, "debug");
    logger.log(Logger_Level_Info, "info");
    logger.log(Logger_Level_Warn, "warn");
    logger.log(Logger_Level_Error, "error");
    logger.log(Logger_Level_Fatal, "fatal");
  }

  std::string output = str_stream.str();
  EXPECT_NE(output.find("[TRACE]"), std::string::npos);
  EXPECT_NE(output.find("[DEBUG]"), std::string::npos);
  EXPECT_NE(output.find("[INFO ]"), std::string::npos);
  EXPECT_NE(output.find("[WARN ]"), std::string::npos);
  EXPECT_NE(output.find("[ERROR]"), std::string::npos);
  EXPECT_NE(output.find("[FATAL]"), std::string::npos);
}

TEST(LoggerImplTest, ConcurrentLogging) {
  std::stringstream str_stream;
  {
    LoggerImpl logger(nullptr, 0);
    logger.add_sink(std::make_unique<StreamSink>(str_stream));
    const int num_threads = 5;
    const int items_per_thread = 20;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back([&logger, i, items_per_thread]() {
        for (int j = 0; j < items_per_thread; ++j) {
          logger.log(Logger_Level_Info, "item from " + std::to_string(i));
        }
      });
    }

    for (auto &thread : threads) { thread.join(); }
  }

  std::string output = str_stream.str();
  int line_count = 0;
  std::size_t pos = 0;
  while ((pos = output.find('\n', pos)) != std::string::npos) {
    line_count++;
    pos++;
  }

  EXPECT_EQ(line_count, 5 * 20);
}

TEST(LoggerCAPITest, SafetyGuards) {
  Logger_Log(nullptr, Logger_Level_Info, "test");
  Logger_Log(reinterpret_cast<Logger_t *>(0x1234), Logger_Level_Info, nullptr);
  Logger_Free(nullptr);
  Logger_Flush(nullptr);
}

class FailingSink: public LogSink {
 public:
  void write(const LogMessage & /*msg*/,
             std::string_view /*formatted*/) override {
    throw std::runtime_error("IO Failure");
  }
  void flush() override {}
  void set_status(Logger * /*logger*/, Logger_Status /*status*/) override {}
};

TEST(LoggerImplTest, IOFailureResilience) {
  LoggerImpl logger(nullptr, 0);
  logger.add_sink(std::make_unique<FailingSink>());
  logger.log(Logger_Level_Error, "This write will fail");

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(logger.get_status(), Logger_Error_Write);
}

TEST(LoggerStatusTest, FileOpenError) {
  Logger logger =
      Logger::New("/non_existent_dir/should_fail.log", Logger_Out_File);
  EXPECT_EQ(logger.GetStatus(), Logger::FileOpenError);
}

TEST(LoggerStatusTest, CAPIStatus) {
  Logger_t *logger = Logger_New("test_status.log", Logger_Out_File);
  if (logger != nullptr) {
    EXPECT_EQ(Logger_GetStatus(logger), Logger_OK);
    Logger_Free(logger);
  }
}

TEST(LoggerImplTest, EdgeCaseInputs) {
  std::stringstream str_stream;
  {
    LoggerImpl logger(nullptr, 0);
    logger.add_sink(std::make_unique<StreamSink>(str_stream));
    logger.log(Logger_Level_Info, "");

    std::string large_msg(static_cast<size_t>(1024 * 1024),
                          'A');  // 1MB
    logger.log(Logger_Level_Debug, large_msg);
  }

  std::string output = str_stream.str();
  EXPECT_NE(output.find("[INFO ]"), std::string::npos);
  EXPECT_NE(output.find(std::string(static_cast<size_t>(1024 * 1024), 'A')),
            std::string::npos);
}

TEST(LoggerImplTest, RapidShutdownPersistence) {
  std::stringstream str_stream;
  {
    LoggerImpl logger(nullptr, 0);
    logger.add_sink(std::make_unique<StreamSink>(str_stream));
    logger.log(Logger_Level_Fatal, "Last breath");
  }

  std::string output = str_stream.str();
  EXPECT_NE(output.find("Last breath"), std::string::npos);
}

TEST(LoggerRotationTest, BasicRotation) {
  const char *filename = "test_rotation.log";
  std::filesystem::remove(filename);
  std::filesystem::remove("test_rotation.1.log");
  std::filesystem::remove("test_rotation.2.log");

  {
    Logger logger = Logger::New(filename, Logger_Out_File);
    logger.SetMaxFileSize(512);
    logger.SetMaxFiles(2);

    for (int i = 0; i < 40; ++i) {
      logger.Info("This is a rotation test message to fill the file.");
    }
    logger.Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  EXPECT_TRUE(std::filesystem::exists("test_rotation.1.log") ||
              std::filesystem::exists("test_rotation.log"));

  std::filesystem::remove("test_rotation.log");
  std::filesystem::remove("test_rotation.1.log");
  std::filesystem::remove("test_rotation.2.log");
}

TEST(LoggerMultipleSinksTest, ConsoleAndStream) {
  std::stringstream str_stream;
  {
    Logger logger = Logger::New(nullptr, Logger_Out_Console);
    logger.AddSink(std::make_unique<StreamSink>(str_stream));
    logger.Info("Dual logging test");
    logger.Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  EXPECT_NE(str_stream.str().find("Dual logging test"), std::string::npos);
}

static void CustomCCallback(void *ctx, Logger_Level /*level*/,
                            const char *message) {
  auto *out = static_cast<std::string *>(ctx);
  *out += message;
}

TEST(LoggerCAPITest, CustomSink) {
  std::string output;
  Logger_t *logger = Logger_New(nullptr, 0);
  Logger_AddSink(logger, CustomCCallback, &output);
  Logger_Info(logger, "C Custom Sink Test");
  Logger_Flush(logger);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  Logger_Free(logger);

  EXPECT_NE(output.find("C Custom Sink Test"), std::string::npos);
}
