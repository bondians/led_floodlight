/******************************************************************************
 * logging.h
 ******************************************************************************/

#ifndef LOGGING_H
#define LOGGING_H

#include <stdint.h>
#include <stdbool.h>
#include "ANSI.h"

// Global debug logging enable
// Setting this to 0 disables most application-generated outputs
// TODO: Need to figure a way to set DEBUG_LOGGING externally (probably via CMakeLists.txt)
// so debug and release builds can be made from the same source.

#if !defined(DEBUG_LOGGING)
#define DEBUG_LOGGING                   1
#endif

// Set LOG_WITH_TIMESTAMP to a nonzero value to enable most logging outputs -
// those invoked using the standard LOG() macro, or DPRINT_TS() - to be prefixed
// with a timestamp derived from the system RTC.

#define LOG_WITH_TIMESTAMP              1

// Debug menu system enable
// Set this to allow the debug menu system to be included.
// Note that DEBUG_LOGGING must be set in addition to this to enable inclusion
// of the debug menu system.

#define DEBUG_MENU                      1

#if DEBUG_LOGGING
  // DPRINTF(...)
  // Unconditional output when the DEBUG_LOGGING build option is enabled.
  // Does not add or modify the output text in any way.
  #define DPRINTF(...) \
          { v_log_printf(__VA_ARGS__); }
  // DPRINTF_TS(...)
  // Unconditional output when the DEBUG_LOGGING build option is enabled, with timestamp
  #define DPRINTF_TS(...) \
          { v_log_printf_ts(__VA_ARGS__); }
  // DPRINTF_C(color, ...)
  // Same as DPRINTF() but with text colorization option
  #define DPRINTF_C(color, ...) { v_logc_printf(color, ##__VA_ARGS__); }

  // LOG_PLAIN(tag, ...)
  // Conditional output (tag != 0) without [TAG] or timestamp prefix text.
  #define LOG_PLAIN(tag, ...) \
          { if (tag) { v_log_printf(__VA_ARGS__); } }
  // LOGC_PLAIN(tag, color, ...)
  // Same as LOG_PLAIN(), but provides option to change foreground color of all
  // text printed by it on an ANSI terminal.
  #define LOGC_PLAIN(tag, color, ...) \
          { if (tag) { v_logc_printf(color, ##__VA_ARGS__); } }
  // LOG(tag, fmt, ...)
  // Conditional output (tag != 0) WITH [TAG] prefix text.
  #define LOG(tag, fmt, ...) \
          { if (tag) { v_log_printf_ts_tag(tag ## _TAG, fmt, ##__VA_ARGS__); } }
  // LOGC(tag, color, fmt, ...)
  // Same as LOG(), but provides option to change foreground color of all text
  // printed by it on an ANSI terminal.
  #define LOGC(tag, color, fmt, ...) \
          { if (tag) { v_logc_printf_ts_tag(tag ## _TAG, color, fmt, ##__VA_ARGS__); } }
#else
  #define DPRINTF(...)
  #define DPRINTF_TS(...)
  #define LOG_PLAIN(tag, ...)
  #define LOGC_PLAIN(tag, color, ...)
  #define LOG(tag, fmt, ...)
  #define LOGC(tag, color, fmt, ...)
#endif

//------------------------------------------------------------------------------

// Color values used with color logging functions
// v_logc_xxx(), LOGC(), LOGC_PLAIN()
// Do not change the order of these enums! The ordinal value of each of these
// must match up with ANSI standard color numbers. See ANSI.h and the
// ANSI_FG_xxxx macro definitions in it to see how this works.

typedef enum __attribute__((packed))
{
    // Don't change the ordering or assigned value for these
    LOGC_BLACK,
    LOGC_RED,
    LOGC_GREEN,
    LOGC_YELLOW,
    LOGC_BLUE,
    LOGC_MAGENTA,
    LOGC_CYAN,
    LOGC_WHITE,
    LOGC_GRAY,
    LOGC_BRIGHT_RED,
    LOGC_BRIGHT_GREEN,
    LOGC_BRIGHT_YELLOW,
    LOGC_BRIGHT_BLUE,
    LOGC_BRIGHT_MAGENTA,
    LOGC_BRIGHT_CYAN,
    LOGC_BRIGHT_WHITE,
    // ANSI attribute bitmasks - these have no enum definition ordering
    // requirement but must be powers of 2 that are >= 0x10
    // (e.g. 0x10, 0x20, 0x40, etc.)
    // These can be bitwise-OR'ed with the color numbers to add text
    // effects. e.g. LOGC_YELLOW | LOGC_BOLD | LOGC_UNDERLINE will produce
    // boldfaced and underlined yellow colored text.
    LOGC_BOLD = 0x10,
    LOGC_UNDERLINE = 0x20,
    LOGC_REVERSE = 0x40,
    LOGC_BLINK = 0x80,
    // Standardized colors for different classes of messages
    // (warning, error, highlight)
    // OK to change these to any color/attribute combination desired.
    LOGC_WARNING = LOGC_YELLOW,
    LOGC_ERROR = LOGC_BRIGHT_RED,
    LOGC_HIGHLIGHT = LOGC_BRIGHT_CYAN,
    // LOGC_NORMAL should have the highest ordinal value +1 of any of the
    // color selections listed here + all attribute bits set.
    // LOGC_BRIGHT_WHITE | LOGC_BOLD | LOGC_UNDERLINE | LOGC_REVERSE |
    // LOGC_BLINK = 0xFF, so LOGC_NORMAL should be set to 0x100.
    // LOGC_NORMAL will have to be changed if new attribute bitmasks are added.
    LOGC_NORMAL = 0x100
}
log_color_t;


//------------------------------------------------------------------------------

extern void v_logging_setup(const char *p_c_tag);

// It is suggested that the logging macros defined above be used to generate
// log messages rather than using these functions directly. These functions
// do not provide any sort of compile-time inclusion filtering, so direct
// calls to these will always be included in the application code, even if
// a release version intended to have no debug outputs is built.

extern bool b_log_lock(void);
extern void v_log_unlock(void);

extern void v_log_printf(char *p_c_format, ...);
extern void v_logc_printf(log_color_t x_color, char *p_c_format, ...);
extern void v_log_printf_ts(char *p_c_format, ...);
extern void v_logc_printf_ts(log_color_t x_color, char *p_c_format, ...);
extern void v_log_printf_ts_tag(char *p_c_tag, char *p_c_format, ...);
extern void v_logc_printf_ts_tag(char *p_c_tag, log_color_t x_color, char *p_c_format, ...);

//------------------------------------------------------------------------------
// Debug enables and associated output tags
//------------------------------------------------------------------------------

// The logging macros such as LOG(ID, format, arg) make use of two macros
// internally - one which determines whether the log statement is placed in the
// code and output, and another, with its symbol name ending in _TAG,
// which provides the text string describing the message classification.
// When invoking a logging macro which uses these filters, only the un-suffixed
// class macro name needs to be provided.
//
// Example:
// If you want to create a new message classification (filter) named
// LOG_FUBAR, you also need to define a macro named LOG_FUBAR_TAG and associate
// it with a string constant such as "FUBAR"
//
// To generate a conditionally-compiled log message using the LOG_FUBAR filter,
// you only need to provide the un-suffixed classification macro name to the
// macro function, like this:
// LOG(LOG_FUBAR, "format string", arg, arg ...)
// Note that you don't have to provide LOG_FUBAR_TAG in the call - the LOG()
// macro will reference it during expansion by concatenating LOG_FUBAR and _TAG
// using the ## macro concatenation operator.

// RTOS and task related
#define LOG_TASK                        1
#define LOG_TASK_TAG                    "TASK"

// Network and WiFi related
#define LOG_NETWORK                     1
#define LOG_NETWORK_TAG                 "NET"

// Non-volatile storage related
#define LOG_NVS                         1
#define LOG_NVS_TAG                     "NVS"

// File system (FFS) related
#define LOG_FILESYS                     1
#define LOG_FILESYS_TAG                 "FILESYS"

// Background operations
#define LOG_BACKGROUND                  1
#define LOG_BACKGROUND_TAG              "BACKGND"

// Miscellaneous - anything that does not fit into one of the above categories
#define LOG_MISC                        1
#define LOG_MISC_TAG                    "MISC"

#endif
