/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libc/unconst.h>

int opterr = 1,	optind = 1, optopt = 0;
char *optarg = 0;

#define	BADOPT	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""

int
getopt(int nargc, char *const nargv[], const char *ostr)
{
  static const char *place = EMSG;	/* option letter processing */
  char *oli;			/* option letter list index */
  char *p;

  if (!*place)
  {
    if (optind >= nargc || *(place = nargv[optind]) != '-')
    {
      place = EMSG;
      return -1;
    }
    if (place[1] && *++place == '-')
    {
      ++optind;
      place = EMSG;
      return -1;
    }
  }

  if ((optopt = (int)*place++) == (int)':'
      || !(oli = strchr(ostr, optopt)))
  {
    /* If the user didn't specify '-' as an option,
       assume it means stop.  */
    if (optopt == (int)'-')
      return -1;
    if (!*place)
      ++optind;
    if (opterr && ostr[0] != ':')
    {
      if (!(p = strrchr(*nargv, '/')))
	p = *nargv;
      else
	++p;
      fprintf(stderr, "%s: illegal option -- %c\n", p, optopt);
    }
    return BADOPT;
  }
  if (*++oli != ':')
  {		/* don't need argument */
    optarg = NULL;
    if (!*place)
      ++optind;
  }
  else
  {				/* need an argument */
    if (*place)			/* no white space */
      optarg = unconst(place, char *);
    else if (nargc <= ++optind)
    { /* no arg */
      place = EMSG;
      if (!(p = strrchr(*nargv, '/')))
	p = *nargv;
      else
	++p;
      if (opterr && ostr[0] != ':')
	fprintf(stderr, "%s: option requires an argument -- %c\n",
		p, optopt);
      /* Posix requires ':' to be returned when it's the first character
         in the option string, else return '?'.  */
      return (ostr[0] == ':') ? BADARG : BADOPT;
    }
    else			/* white space */
      optarg = nargv[optind];
    place = EMSG;
    ++optind;
  }
  return optopt;		/* dump back option letter */
}
