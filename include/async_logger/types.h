#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Log levels for the Logger.
 */
typedef enum {
  Logger_Level_Trace = 0,
  Logger_Level_Debug,
  Logger_Level_Info,
  Logger_Level_Warn,
  Logger_Level_Error,
  Logger_Level_Fatal,
  Logger_Level_Off
} Logger_Level;

/**
 * @brief Status values for the Logger.
 */
typedef enum {
  Logger_OK = 0,
  Logger_Error_File_Open,
  Logger_Error_Write,
  Logger_Error_Uninitialized
} Logger_Status;

/**
 * @brief Output destination flags.
 */
typedef enum {
  Logger_Out_None = 0,
  Logger_Out_Console = 1 << 0,
  Logger_Out_File = 1 << 1
} Logger_Sink_Flags;

#ifdef __cplusplus
}
#endif
