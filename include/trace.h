#ifndef SCRIBE_TRACE_H
#define SCRIBE_TRACE_H

#include <TracyC.h>
#include <log.h>

#define INIT_TRACE TracyCZoneCtx ctx

#define START_ZONE TracyCZoneS(ctx, 10, 1)
#define END_ZONE TracyCZoneEnd(ctx)

#define MESSAGE_TRACE(TXT) TracyCMessageLC((TXT), 0xffffff)
#define MESSAGE_DEBUG(TXT) TracyCMessageLC((TXT), 0xff1493)
#define MESSAGE_INFO(TXT) TracyCMessageLC((TXT), 0x00ff00)
#define MESSAGE_WARN(TXT) TracyCMessageLC((TXT), 0xffa500)
#define MESSAGE_ERROR(TXT) TracyCMessageLC((TXT), 0xff6347)
#define MESSAGE_FATAL(TXT) TracyCMessageLC((TXT), 0xff0000)

#define RECORD_VALUE(NAME, VALUE) TracyCPlot((NAME), (VALUE))

#define MARK_FRAME(NAME) TracyCFrameMarkNamed((NAME))

#define message_trace(TXT) \
  MESSAGE_TRACE((TXT));    \
  log_trace((TXT));

#define message_debug(TXT) \
  MESSAGE_DEBUG((TXT));    \
  log_debug((TXT));

#define message_info(TXT) \
  MESSAGE_INFO((TXT));    \
  log_info((TXT));

#define message_warn(TXT) \
  MESSAGE_WARN((TXT));    \
  log_warn((TXT));

#define message_error(TXT) \
  MESSAGE_ERROR((TXT));    \
  log_error((TXT));

#define message_fatal(TXT) \
  MESSAGE_FATAL((TXT));    \
  log_fatal((TXT));

#endif  // SCRIBE_TRACE_H
