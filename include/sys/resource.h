/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_resource_h_
#define __dj_include_sys_resource_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#include <time.h>

#define RUSAGE_SELF     0               /* calling process */
#define RUSAGE_CHILDREN -1              /* terminated child processes */

struct rusage {
  struct timeval ru_utime;	/* user time used */
  struct timeval ru_stime;	/* system time used */
  long ru_maxrss;		/* integral max resident set size */
  long ru_ixrss;		/* integral shared text memory size */
  long ru_idrss;		/* integral unshared data size */
  long ru_isrss;		/* integral unshared stack size */
  long ru_minflt;		/* page reclaims */
  long ru_majflt;		/* page faults */
  long ru_nswap;		/* swaps */
  long ru_inblock;		/* block input operations */
  long ru_oublock;		/* block output operations */
  long ru_msgsnd;		/* messages sent */
  long ru_msgrcv;		/* messages received */
  long ru_nsignals;		/* signals received */
  long ru_nvcsw;		/* voluntary context switches */
  long ru_nivcsw;		/* involuntary context switches */
};

int getrusage(int _who, struct rusage *_rusage);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_resource_h_ */
