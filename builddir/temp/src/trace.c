#include "trace.h"

#include <log.h>

void message_trace(char const* txt) {
  log_trace(txt);
  MESSAGE_TRACE(txt);
}

void message_debug(char const* txt) {
  log_debug(txt);
  MESSAGE_DEBUG(txt);
}

void message_info(char const* txt) {
  log_info(txt);
  MESSAGE_INFO(txt);
}

void message_warn(char const* txt) {
  log_warn(txt);
  MESSAGE_WARN(txt);
}

void message_error(char const* txt) {
  log_error(txt);
  MESSAGE_ERROR(txt);
}

void message_fatal(char const* txt) {
  log_fatal(txt);
  MESSAGE_FATAL(txt);
}