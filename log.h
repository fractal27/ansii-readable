#ifndef VSD_LOG_H
#define VSD_LOG_H
#include <stdio.h>


#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define COLOR_VERBOSE "\e[35m"
#define COLOR_INFO "\e[34m"
#define COLOR_NOTICE "\e[34m"
#define COLOR_WARN "\e[33m"
#define COLOR_ERROR "\e[31m"
#define COLOR_RESET "\e[0m"

#define log_verbose(format, ...) log_generic(LOG_VERBOSE,"\e[0m" __BASE_FILE__  ":%16s (" COLOR_VERBOSE "%07s" COLOR_RESET ")  " format, __func__, str_log_level(LOG_VERBOSE) __VA_OPT__(,) __VA_ARGS__)
#define log_info(format, ...)    log_generic(LOG_INFO,"\e[0m"    __BASE_FILE__  ":%16s (" COLOR_INFO    "%07s" COLOR_RESET ")  " format, __func__, str_log_level(LOG_INFO) __VA_OPT__(,) __VA_ARGS__)
#define log_notice(format, ...)  log_generic(LOG_NOTICE,"\e[0m"  __BASE_FILE__  ":%16s (" COLOR_NOTICE  "%07s" COLOR_RESET ")  " format, __func__, str_log_level(LOG_NOTICE) __VA_OPT__(,) __VA_ARGS__)
#define log_warning(format, ...) log_generic(LOG_WARN,"\e[0m"    __BASE_FILE__  ":%16s (" COLOR_WARN    "%07s" COLOR_RESET ")  " format, __func__, str_log_level(LOG_WARN) __VA_OPT__(,) __VA_ARGS__)
#define log_error(format, ...)   log_generic(LOG_ERROR,"\e[0m"   __BASE_FILE__  ":%16s (" COLOR_ERROR   "%07s" COLOR_RESET ")  " format, __func__, str_log_level(LOG_ERROR) __VA_OPT__(,) __VA_ARGS__)

enum log_level{
       LOG_VERBOSE = 0,
       LOG_INFO    = 10,
       LOG_NOTICE  = 20,
       LOG_WARN    = 30,
       LOG_ERROR   = 40,
};

extern enum log_level log_level;
extern FILE* stdlog;

FILE* get_logfile();
void log_generic(enum log_level log_level, const char* fmt, ...);
const char* str_log_level(enum log_level log_level);

#endif // VSD_LOG_H
