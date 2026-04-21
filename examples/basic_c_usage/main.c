#include <logger.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

int main() {
  Logger *logger =
      Logger_New("output-c.log", Logger_Out_File | Logger_Out_Console);
  if (logger == NULL) {
    fprintf(stderr, "Error: Could not create logger instance.\n");
    return 1;
  }

  Logger_Info(logger, "C Application started.");
  Logger_Debug(logger, "Internal state initialized.");

  for (int index = 1; index <= 5; ++index) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Processing data packet #%d", index);
    Logger_Info(logger, buffer);

    if (index == 3) {
      Logger_Warn(logger, "Detected high latency in packet processing.");
    }

    SLEEP_MS(200);
  }

  Logger_Info(logger, "Application shutting down cleanly.");

  Logger_Free(logger);

  return 0;
}
