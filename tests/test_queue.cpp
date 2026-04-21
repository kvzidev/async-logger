#include <gtest/gtest.h>

#include <stop_token>
#include <thread>
#include <vector>

#include "../src/synchronized_queue.hpp"

using namespace async_logger;

TEST(SynchronizedQueueTest, SimplePushPop) {
  SynchronizedQueue queue;
  LogMessage msg(Logger_Level_Info, "test message");

  queue.push(std::move(msg));
  EXPECT_FALSE(queue.empty());

  std::stop_source source;
  auto popped = queue.pop(source.get_token());

  ASSERT_TRUE(popped.has_value());
  if (popped.has_value()) {
    EXPECT_EQ(popped.value().m_content, "test message");
    EXPECT_EQ(popped.value().m_level, Logger_Level_Info);
  }
  EXPECT_TRUE(queue.empty());
}

TEST(SynchronizedQueueTest, StopTokenInterruptsPop) {
  SynchronizedQueue queue;
  std::stop_source source;

  std::thread joiner([&queue, &source]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    source.request_stop();
  });

  auto start = std::chrono::steady_clock::now();
  auto popped = queue.pop(source.get_token());
  auto end = std::chrono::steady_clock::now();

  EXPECT_FALSE(popped.has_value());
  EXPECT_GE(end - start, std::chrono::milliseconds(100));

  if (joiner.joinable()) { joiner.join(); }
}

TEST(SynchronizedQueueTest, ConcurrentPush) {
  SynchronizedQueue queue;
  const int num_threads = 10;
  const int items_per_thread = 100;

  std::vector<std::thread> producers;
  producers.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    producers.emplace_back([&queue, i, items_per_thread]() {
      for (int j = 0; j < items_per_thread; ++j) {
        queue.push(LogMessage(Logger_Level_Debug, "msg"));
      }
    });
  }

  for (auto &thread : producers) { thread.join(); }

  int count = 0;
  std::stop_source source;
  while (!queue.empty()) {
    auto popped = queue.pop(source.get_token());
    if (popped) { count++; }
  }

  EXPECT_EQ(count, num_threads * items_per_thread);
}
