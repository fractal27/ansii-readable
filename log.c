#include "log.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

FILE* stdlog;
bool stdlog_initialized;

enum log_level log_level = LOG_NOTICE;


inline FILE*
get_logfile(){
       return stdlog_initialized?
              stdlog:
              stderr;
}

const char* 
str_log_level(enum log_level log_level){
       switch(log_level){
              case LOG_VERBOSE: return "verbose";
              case LOG_INFO:    return "info";
              case LOG_NOTICE:  return "notice";
              case LOG_WARN:    return "warn";
              case LOG_ERROR:   return "error";
       }
}

void 
log_generic(enum log_level logging_level, const char* fmt, ...){
       va_list args;

       if(logging_level >= log_level){
              va_start(args,fmt);
              vfprintf(get_logfile(), fmt, args);
              va_end(args);
       }
       return;
}


