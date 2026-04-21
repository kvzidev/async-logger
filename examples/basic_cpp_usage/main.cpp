#include <chrono>
#include <iostream>
#include <logger>
#include <thread>

int main() {
  async_logger::Logger logger = async_logger::Logger::New(
      "output-cpp.log", Logger_Out_File | Logger_Out_Console);

  if (logger.GetStatus() != async_logger::Status::Logger_OK) {
    std::cerr << "Error: Could not create logger instance.\n";
    return 1;
  }

  logger.Info("Application started accurately.");
  logger.Debug("Internal state initialized.");

  for (int index = 1; index <= 5; ++index) {
    std::string msg = "Processing data packet #" + std::to_string(index);
    logger.Info(msg.c_str());

    if (index == 3) { logger.Warn("Packet #3 requires special handling."); }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  logger.Error("Simulated data processing error occurred.");
  logger.Fatal("Application must terminate due to critical error.");

  return 0;
}
