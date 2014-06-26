#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(DOS)
#if defined(_MSC_VER)
# if defined(SHARING)        /* Import defines for locking regions  StH */
#  include <sys/locking.h>
# endif
#endif    /* _MSC_VER */
#endif    /* DOS */

#define PYMSG

#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "platform/pytime.h"
#include "debugging/assert.h"

typedef unsigned int UnInt;
typedef unsigned char UnChar;
typedef unsigned long UnLong;


#if !defined(SEEK_SET)
#    define SEEK_SET 0
#endif    /* not SEEK_SET */

static char const * const *ActualMsgTab;

static message_id_t StringCnt;

/* This is used to record an argument which is used
 * in a message-string. There are only message strings
 * that contain at most one format specifier.  Therefore
 * one pointer is sufficient.
 * Three small routines are provided to assign a value:
 */

boolean InitMsgTab(Language l)
{
  StringCnt = MsgCount;
  ActualMsgTab = MessageTabs[l];
  return true;
}

#if defined(DEBUG)
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

void Message(message_id_t id, ...)
{
  DBG((stderr, "Mesage(%d) = %s\n", id, ActualMsgTab[id]));

  if (id<StringCnt)
  {
    va_list args;
    va_start(args,id);
    protocol_vfprintf(stdout,ActualMsgTab[id],args);
    va_end(args);
  }
  else
    Message(InternalError,id);
}

void Message2(FILE *file, message_id_t id, ...)
{
  va_list args;
  DBG((stderr, "Mesage(%d) = %s\n", id, ActualMsgTab[id]));
  va_start(args,id);
  if (id<StringCnt)
    vfprintf(file,ActualMsgTab[id],args);
  else
    fprintf(file,ActualMsgTab[InternalError],id);
  va_end(args);
}

void ErrorMsg(message_id_t id, ...)
{
  DBG((stderr, "ErrorMsg(%d) = %s\n", id, ActualMsgTab[id]));
#if !defined(QUIET)
  if (id<StringCnt)
  {
    va_list args;
    va_start(args,id);
    protocol_vfprintf(stderr,ActualMsgTab[id],args);
    va_end(args);
  }
  else
    ErrorMsg(InternalError,id);

  protocol_fflush(stderr);
#endif
}

void FtlMsg(message_id_t id)
{
  ErrorMsg(ErrFatal);
  ErrorMsg(NewLine);
  ErrorMsg(id);
  ErrorMsg(NewLine);
  exit(id);
}

void VerifieMsg(message_id_t id)
{
  ErrorMsg(id);
  ErrorMsg(NewLine);
  ErrorMsg(ProblemIgnored);
  ErrorMsg(NewLine);
}

static void ErrChar(char c)
{
#if !defined(QUIET)
  protocol_fputc(c,stderr);
  protocol_fflush(stderr);
#endif
}

void IoErrorMsg(message_id_t n, int val)
{
  fflush(stdout);
  ErrorMsg(InputError,val);
  ErrorMsg(n);
  ErrChar('\n');
  ErrorMsg(OffendingItem,InputLine);
  ErrChar('\n');
}

static void FormatTime(FILE *file)
{
  unsigned long msec;
  unsigned long secs;
  StopTimer(&secs,&msec);

  {
    unsigned long const Hours = secs/3600;
    unsigned long const Minutes = (secs%3600)/60;
    unsigned long const Seconds = (secs%60);

    if (Hours>0)
      fprintf(file,"%lu:%02lu:%02lu h:m:s",Hours,Minutes,Seconds);
    else if (Minutes>0)
    {
      if (msec==MSEC_NOT_SUPPORTED)
        fprintf(file,"%lu:%02lu m:s", Minutes, Seconds);
      else
        fprintf(file,"%lu:%02lu.%03lu m:s", Minutes, Seconds, msec);
    }
    else
    {
      if (msec==MSEC_NOT_SUPPORTED)
        fprintf(file,"%lu s", Seconds);
      else
        fprintf(file,"%lu.%03lu s", Seconds, msec);
    }
  }
}

void PrintTime(FILE *file, char const *header, char const *trail)
{
  if (!flag_regression)
  {
    fprintf(file,"%s",header);
    fprintf(file,ActualMsgTab[TimeString]);
    FormatTime(file);
    fprintf(file,"%s",trail);
  }
}

static void FormatTime1(void)
{
  unsigned long msec;
  unsigned long secs;
  StopTimer(&secs,&msec);

  {
    unsigned long const Hours = secs/3600;
    unsigned long const Minutes = (secs%3600)/60;
    unsigned long const Seconds = (secs%60);

    if (Hours>0)
      protocol_fprintf(stdout,"%lu:%02lu:%02lu h:m:s",Hours,Minutes,Seconds);
    else if (Minutes>0)
    {
      if (msec==MSEC_NOT_SUPPORTED)
        protocol_fprintf(stdout,"%lu:%02lu m:s", Minutes, Seconds);
      else
        protocol_fprintf(stdout,"%lu:%02lu.%03lu m:s", Minutes, Seconds, msec);
    }
    else
    {
      if (msec==MSEC_NOT_SUPPORTED)
        protocol_fprintf(stdout,"%lu s", Seconds);
      else
        protocol_fprintf(stdout,"%lu.%03lu s", Seconds, msec);
    }
  }
}

void PrintTime1(char const *header, char const *trail)
{
  if (!flag_regression)
  {
    protocol_fprintf(stdout,"%s",header);
    protocol_fprintf(stdout,ActualMsgTab[TimeString]);
    FormatTime1();
    protocol_fprintf(stdout,"%s",trail);
  }
}

void ReportAborted(int signal)
{
  protocol_fputc('\n',stdout);
  protocol_fprintf(stdout,ActualMsgTab[Abort],signal);
  FormatTime1();
  protocol_fputc('\n',stdout);
}
