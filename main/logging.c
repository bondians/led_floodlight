/******************************************************************************
 * logging.c
 ******************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "esp_log.h"

#include "logging.h"

//-------------------------------------------------------------------------------

// Including and using these functions is probably not intended, as they are
// implied to be private; they are defined in esp-idf/components/log/esp_log_private.h.
// However, making use of them allows the application logging functions to share
// the same mutex that is used by the ESP-IDF logging functions, thus preventing
// outputs from esp-idf logging and application logging from stepping on each other.
// Their definitions and calling convention are unlikely to change.

//void esp_log_impl_lock(void);
bool esp_log_impl_lock_timeout(void);
void esp_log_impl_unlock(void);

/******************************************************************************
 *
 * Notes:
 * Logging filters affect only those outputs that are done using the ESP_LOGx()
 * macros; e.g. ESP_LOGI(), ESP_LOGE(), etc.
 * The menuconfig/sdkconfig build configuration defines a maximum verbosity
 * level for log messages. Messages with a verbosity level higher than this
 * setting will not be built into the code at compile time.
 ******************************************************************************/

void v_logging_setup(const char *p_c_tag)
{
    // Set logging level for all modules to INFO level by default

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(p_c_tag, ESP_LOG_INFO);
    esp_log_level_set("HTTP_CLIENT", ESP_LOG_DEBUG);
}

/*******************************************************************************
 *
 *******************************************************************************/

bool b_log_lock(void)
{
    return esp_log_impl_lock_timeout();
}

void v_log_unlock(void)
{
    esp_log_impl_unlock();
}

/*******************************************************************************
 *
 *******************************************************************************/

static void v_print_color(log_color_t x_color)
{
    if (x_color == LOGC_NORMAL)
    {
        printf(ANSI_NORMAL);
    }
    else
    {
        printf(ANSI_FG_FMT "%s%s%s%s",
               x_color & 0x0F,
               ((x_color & LOGC_BOLD) ? ANSI_BOLD : ""),
               ((x_color & LOGC_UNDERLINE) ? ANSI_UNDERLINE : ""),
               ((x_color & LOGC_REVERSE) ? ANSI_REVERSE : ""),
               ((x_color & LOGC_BLINK) ? ANSI_BLINK : "") );
    }
}

/*******************************************************************************
 *
 *******************************************************************************/

void v_log_printf(char *p_c_format, ...)
{
    va_list args;

    if (!b_log_lock())
    {
        return;
    }

    va_start(args, p_c_format);
    vprintf(p_c_format, args);
    va_end(args);

    v_log_unlock();
}

/*******************************************************************************
 *
 *******************************************************************************/

void v_logc_printf(log_color_t x_color, char *p_c_format, ...)
{
    va_list args;

    if (!b_log_lock())
    {
        return;
    }

    v_print_color(x_color);

    va_start(args, p_c_format);
    vprintf(p_c_format, args);
    va_end(args);

    v_print_color(LOGC_NORMAL);

    v_log_unlock();
}

/*******************************************************************************
 *
 *******************************************************************************/

void v_log_printf_ts(char *p_c_format, ...)
{
    va_list args;

    if (!b_log_lock())
    {
        return;
    }

    #if LOG_WITH_TIMESTAMP
    char *p_c_timestamp = esp_log_system_timestamp();
    printf("(%s%) ", p_c_timestamp);
    #endif

    va_start(args, p_c_format);
    vprintf(p_c_format, args);
    va_end(args);

    putchar('\n');

    v_log_unlock();
}

/*******************************************************************************
 *
 *******************************************************************************/

void v_logc_printf_ts(log_color_t x_color, char *p_c_format, ...)
{
    va_list args;

    if (!b_log_lock())
    {
        return;
    }

    v_print_color(x_color);

    #if LOG_WITH_TIMESTAMP
    char *p_c_timestamp = esp_log_system_timestamp();
    printf("(%s%) ", p_c_timestamp);
    #endif

    va_start(args, p_c_format);
    vprintf(p_c_format, args);
    va_end(args);

    v_print_color(LOGC_NORMAL);
    putchar('\n');

    v_log_unlock();
}

/*******************************************************************************
 *
 *******************************************************************************/

void v_log_printf_ts_tag(char *p_c_tag, char *p_c_format, ...)
{
    va_list args;

    if (!b_log_lock())
    {
        return;
    }

    #if LOG_WITH_TIMESTAMP
    char *p_c_timestamp = esp_log_system_timestamp();
    printf("(%s%) [%s] ", p_c_timestamp, p_c_tag);
    #else
    printf("[%s] ", p_c_tag);
    #endif

    va_start(args, p_c_format);
    vprintf(p_c_format, args);
    va_end(args);

    putchar('\n');

    v_log_unlock();
}

/*******************************************************************************
 *
 *******************************************************************************/

void v_logc_printf_ts_tag(char *p_c_tag, log_color_t x_color, char *p_c_format, ...)
{
    va_list args;

    if (!b_log_lock())
    {
        return;
    }

    v_print_color(x_color);

    #if LOG_WITH_TIMESTAMP
    char *p_c_timestamp = esp_log_system_timestamp();
    printf("(%s%) [%s] ", p_c_timestamp, p_c_tag);
    #else
    printf("[%s] ", p_c_tag);
    #endif

    va_start(args, p_c_format);
    vprintf(p_c_format, args);
    va_end(args);

    v_print_color(LOGC_NORMAL);
    putchar('\n');

    v_log_unlock();
}
