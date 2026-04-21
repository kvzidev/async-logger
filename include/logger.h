#pragma once

#include <async_logger/custom.h>
#include <async_logger/types.h>
#include <stddef.h>
#include <stdint.h>

#define ASYNC_LOGGER_LEVEL_TRACE 0
#define ASYNC_LOGGER_LEVEL_DEBUG 1
#define ASYNC_LOGGER_LEVEL_INFO  2
#define ASYNC_LOGGER_LEVEL_WARN  3
#define ASYNC_LOGGER_LEVEL_ERROR 4
#define ASYNC_LOGGER_LEVEL_FATAL 5
#define ASYNC_LOGGER_LEVEL_OFF   6

#ifndef ASYNC_LOGGER_MIN_LEVEL
#define ASYNC_LOGGER_MIN_LEVEL ASYNC_LOGGER_LEVEL_TRACE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to the logger instance.
 */
#ifdef __cplusplus
typedef struct LoggerHandle Logger_t;
#else
typedef struct LoggerHandle Logger;
typedef Logger Logger_t;
#endif

/**
 * @brief Creates a new logger instance.
 */
Logger_t *Logger_New(const char *filename, int flags);

/**
 * @brief Adds a custom sink using a C callback.
 */
void Logger_AddSink(Logger_t *logger, Logger_Sink_Callback callback, void *ctx);

/**
 * @brief Sets a custom formatter using a C callback.
 */
void Logger_SetFormatter(Logger_t *logger, Logger_Formatter_Callback callback,
                         void *ctx);

/**
 * @brief Destroys the logger instance and flushes pending messages.
 */
void Logger_Free(Logger_t *logger);

/**
 * @brief Submits a log message asynchronously.
 */
void Logger_Log(Logger_t *logger, Logger_Level level, const char *message);

/**
 * @brief Flushes all pending log messages to disk.
 */
void Logger_Flush(Logger_t *logger);

/**
 * @brief Returns the current status of the logger.
 */
Logger_Status Logger_GetStatus(Logger_t *logger);

/**
 * @brief Forces a new status for the logger.
 */
void Logger_SetStatus(Logger_t *logger, Logger_Status status);

/**
 * @brief Sets the maximum file size before rotation occurs.
 */
void Logger_SetMaxFileSize(Logger_t *logger, size_t size_bytes);

/**
 * @brief Sets the maximum number of rotated files to keep.
 */
void Logger_SetMaxFiles(Logger_t *logger, int count);

static inline void Logger_Trace(Logger_t *logger, const char *message) {
#if ASYNC_LOGGER_MIN_LEVEL <= ASYNC_LOGGER_LEVEL_TRACE
  Logger_Log(logger, Logger_Level_Trace, message);
#else
  (void)logger;
  (void)message;
#endif
}

static inline void Logger_Debug(Logger_t *logger, const char *message) {
#if ASYNC_LOGGER_MIN_LEVEL <= ASYNC_LOGGER_LEVEL_DEBUG
  Logger_Log(logger, Logger_Level_Debug, message);
#else
  (void)logger;
  (void)message;
#endif
}

static inline void Logger_Info(Logger_t *logger, const char *message) {
#if ASYNC_LOGGER_MIN_LEVEL <= ASYNC_LOGGER_LEVEL_INFO
  Logger_Log(logger, Logger_Level_Info, message);
#else
  (void)logger;
  (void)message;
#endif
}

static inline void Logger_Warn(Logger_t *logger, const char *message) {
#if ASYNC_LOGGER_MIN_LEVEL <= ASYNC_LOGGER_LEVEL_WARN
  Logger_Log(logger, Logger_Level_Warn, message);
#else
  (void)logger;
  (void)message;
#endif
}

static inline void Logger_Error(Logger_t *logger, const char *message) {
#if ASYNC_LOGGER_MIN_LEVEL <= ASYNC_LOGGER_LEVEL_ERROR
  Logger_Log(logger, Logger_Level_Error, message);
#else
  (void)logger;
  (void)message;
#endif
}

static inline void Logger_Fatal(Logger_t *logger, const char *message) {
#if ASYNC_LOGGER_MIN_LEVEL <= ASYNC_LOGGER_LEVEL_FATAL
  Logger_Log(logger, Logger_Level_Fatal, message);
#else
  (void)logger;
  (void)message;
#endif
}

#ifdef __cplusplus
}
#endif
