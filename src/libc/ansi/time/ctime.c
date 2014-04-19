/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2005 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This file has been modified by DJ Delorie.  These modifications are
** Copyright (C) 1995 DJ Delorie, 334 North Rd, Deerfield NH 03037-1110
** 03867-2954, USA.
*/

/*
 * Copyright (c) 1987, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Arthur David Olson of the National Cancer Institute.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ctime.c	5.23 (Berkeley) 6/22/90";
#endif /* LIBC_SCCS and not lint */

/*
** Leap second handling from Bradley White (bww@k.gp.cs.cmu.edu).
** POSIX-style TZ environment variable handling from Guy Harris
** (guy@auspex.com).
*/

#include <libc/stubs.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <tzfile.h>

#include <libc/unconst.h>
#include <libc/bss.h>
#include <libc/environ.h>

#include "posixrul.h"

#if ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)) || (__GNUC__ >= 3)
# define ATTRIBUTE_CONST __attribute__ ((const))
# define ATTRIBUTE_PURE  __attribute__ ((__pure__))
#else
# define ATTRIBUTE_CONST /* empty */
# define ATTRIBUTE_PURE  /* empty */
#endif

#define GRANDPARENTED        "Local time zone must be set--see zic manual page"
#define TZ_ABBR_MAX_LEN      16
#define TZ_ABBR_CHAR_SET     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 :+-._"
#define TZ_ABBR_ERR_CHAR     '_'

#define TYPE_BIT(type)       (sizeof (type) * CHAR_BIT)
#define TYPE_SIGNED(type)    (((type) -1) < 0)
#define TYPE_INTEGRAL(type)  (((type) 0.5) != 0.5)

#define AVGSECSPERYEAR       31556952L   /* The Gregorian year averages 365.2425 days, which is 31556952 seconds.  */
#define YEARSPERREPEAT       400         /* Years before a Gregorian repeat */
#define SECSPERREPEAT        ((int_fast64_t) YEARSPERREPEAT * (int_fast64_t) AVGSECSPERYEAR)
#define SECSPERREPEAT_BITS   34          /* ceil(log2(SECSPERREPEAT)) */

#define ACCESS_MODE          (O_RDONLY|O_BINARY)
#define OPEN_MODE            (O_RDONLY|O_BINARY)
#define IS_ABSOLUTE(n)       ((n)[0] == '/' || (n)[0] == '\\' || (n)[1] == ':')
#define INITIALIZE(x)        ((x) = 0)

/* Unlike <ctype.h>'s isdigit, this also works if c < 0 | c > UCHAR_MAX. */
#define is_digit(c)          ((unsigned)(c) - '0' <= 9)


/*
** Someone might make incorrect use of a time zone abbreviation:
**      1.      They might reference tzname[0] before calling tzset (explicitly
**              or implicitly).
**      2.      They might reference tzname[1] before calling tzset (explicitly
**              or implicitly).
**      3.      They might reference tzname[1] after setting to a time zone
**              in which Daylight Saving Time is never observed.
**      4.      They might reference tzname[0] after setting to a time zone
**              in which Standard Time is never observed.
**      5.      They might reference tm.TM_ZONE after calling offtime.
** What's best to do in the above cases is open to debate;
** for now, we just set things up so that in any of the five cases
** WILDABBR is used.  Another possibility:  initialize tzname[0] to the
** string "tzname[0] used before set", and similarly for the other cases.
** And another:  initialize tzname[0] to "ERA", with an explanation in the
** manual page of what this "time zone abbreviation" means (doing this so
** that tzname[0] has the "normal" length of three characters).
*/
static char wildabbr[] = "   ";

#ifndef TRUE
#define TRUE   1
#define FALSE  0
#endif /* !defined TRUE */

static const char gmt[] = "GMT";

/*
** The DST rules to use if TZ has no rules and we can't load TZDEFRULES.
** We default to US rules as of 1999-08-17.
** POSIX 1003.1 section 8.1.1 says that the default DST rules are
** implementation dependent; for historical reasons, US rules are a
** common default.
*/
#ifndef TZDEFRULESTRING
#define TZDEFRULESTRING ",M4.1.0,M10.5.0"
#endif /* !defined TZDEFDST */

struct ttinfo {             /* time type information */
  int_fast32_t tt_gmtoff;   /* UTC offset in seconds */
  int tt_isdst;             /* used to set tm_isdst */
  int tt_abbrind;           /* abbreviation list index */
  int tt_ttisstd;           /* TRUE if transition is std time */
  int tt_ttisgmt;           /* TRUE if transition is UTC */
};

struct lsinfo {             /* leap second information */
  time_t ls_trans;          /* transition time */
  int_fast64_t ls_corr;     /* correction to apply */
};

#define BIGGEST(a, b)   (((a) > (b)) ? (a) : (b))

#ifdef TZNAME_MAX
#define MY_TZNAME_MAX   TZNAME_MAX
#endif /* defined TZNAME_MAX */
#ifndef TZNAME_MAX
#define MY_TZNAME_MAX   255
#endif /* !defined TZNAME_MAX */

struct state {
  int leapcnt;
  int timecnt;
  int typecnt;
  int charcnt;
  int goback;
  int goahead;
  time_t ats[TZ_MAX_TIMES];
  unsigned char types[TZ_MAX_TIMES];
  struct ttinfo ttis[TZ_MAX_TYPES];
  char chars[BIGGEST(BIGGEST(TZ_MAX_CHARS + 1, sizeof gmt), (2 * (MY_TZNAME_MAX + 1)))];
  struct lsinfo lsis[TZ_MAX_LEAPS];
  int defaulttype;  /* for early times or if no transitions */
};

struct rule {
  int r_type;           /* type of rule--see below */
  int r_day;            /* day number of rule */
  int r_week;           /* week number of rule */
  int r_mon;            /* month number of rule */
  int_fast32_t r_time;  /* transition time of rule */
};

#define JULIAN_DAY             0        /* Jn - Julian day */
#define DAY_OF_YEAR            1        /* n - day of year */
#define MONTH_NTH_DAY_OF_WEEK  2        /* Mm.n.d - month, week, day of week */

/*
** Prototypes for static functions.
*/

static int_fast32_t  detzcode(const char *codep);
static int           differ_by_repeat(time_t t1, time_t t0);
static const char   *getnum(const char *strp, int *nump, int min, int max);
static const char   *getqzname(const char * strp, const int delim) ATTRIBUTE_PURE;
static const char   *getzname(const char *strp) ATTRIBUTE_PURE;
static const char   *getsecs(const char *strp, int_fast32_t *const secsp);
static const char   *getoffset(const char *strp, int_fast32_t *const offsetp);
static const char   *getrule(const char *strp, struct rule *rulep);
static void          gmtload(struct state *sp);
static struct tm    *gmtsub(const time_t *const timep, const int_fast32_t offset, struct tm *const tmp);
static int           increment_overflow(int *const ip, int j);
static struct tm    *localsub(const time_t *const timep, const int_fast32_t offset, struct tm *const tmp);
static void          settzname(void);
static time_t        time1(struct tm *const tmp, struct tm *(*const funcp) (const time_t *, int_fast32_t, struct tm *), const int_fast32_t offset);
static time_t        time2(struct tm *const tmp, struct tm *(*const funcp)(const time_t *, int_fast32_t, struct tm *), const int_fast32_t offset, int *const okayp);
static struct tm    *timesub(const time_t *const timep, const int_fast32_t offset, register const struct state *const sp, register struct tm *const tmp);
static int           tmcomp(const struct tm *atmp, const struct tm *btmp);
static time_t        transtime(time_t janfirst, int year, const struct rule *rulep, int_fast32_t offset) ATTRIBUTE_PURE;
static int           typesequiv(const struct state *const sp, const int a, const int b);
static int           tzload(const char *name, struct state *const sp, const int doextend);
static int           tzparse(const char *name, struct state *sp, int lastditch);

static struct state lclmem;
static struct state gmtmem;
#define lclptr (&lclmem)
#define gmtptr (&gmtmem)

#ifndef TZ_STRLEN_MAX
#define TZ_STRLEN_MAX  255
#endif /* !defined TZ_STRLEN_MAX */

static char lcl_TZname[TZ_STRLEN_MAX + 1];
static int lcl_is_set;  /* 0: no, 1: set by tzset, -1: set by tzsetwall */
static int gmt_is_set;

char *tzname[2] = {
  wildabbr,
  wildabbr
};

static int_fast32_t
detzcode(const char *const codep)
{
  register int_fast32_t result;
  register int i;

  result = (codep[0] & 0x80) ? -1 : 0;
  for (i = 0; i < 4; ++i)
    result = (result << 8) | (codep[i] & 0xff);
  return result;
}

static void
settzname(void)
{
  register struct state *const sp = lclptr;
  int i;

  tzname[0] = wildabbr;
  tzname[1] = wildabbr;

  /*
   ** And to get the latest zone names into tzname. . .
   */
  for (i = 0; i < sp->typecnt; ++i)
  {
    register const struct ttinfo *const ttisp = &sp->ttis[i];

    tzname[ttisp->tt_isdst] = unconst(&sp->chars[ttisp->tt_abbrind], char *);
  }
  for (i = 0; i < sp->timecnt; ++i)
  {
    register const struct ttinfo *const ttisp = &sp->ttis[sp->types[i]];

    tzname[ttisp->tt_isdst] = unconst(&sp->chars[ttisp->tt_abbrind], char *);
  }
  /*
  ** Finally, scrub the abbreviations.
  ** First, replace bogus characters.
  */
  for (i = 0; i < sp->charcnt; ++i)
    if (strchr(TZ_ABBR_CHAR_SET, sp->chars[i]) == NULL)
      sp->chars[i] = TZ_ABBR_ERR_CHAR;
  /*
  ** Second, truncate long abbreviations.
  */
  for (i = 0; i < sp->typecnt; ++i)
  {
    register const struct ttinfo *const ttisp = &sp->ttis[i];
    register char *cp = unconst(&sp->chars[ttisp->tt_abbrind], char *);
    if (strlen(cp) > TZ_ABBR_MAX_LEN && strcmp(cp, GRANDPARENTED) != 0)
      *(cp + TZ_ABBR_MAX_LEN) = '\0';
  }
}

static int
differ_by_repeat(const time_t t1, const time_t t0)
{
  if (TYPE_INTEGRAL(time_t) && TYPE_BIT(time_t) - TYPE_SIGNED(time_t) < SECSPERREPEAT_BITS)
    return 0;
  return (int_fast64_t)t1 - (int_fast64_t)t0 == SECSPERREPEAT;
}

static char *
tzdir(void)
{
  static char *cp, dir[FILENAME_MAX + 1] = {0};
  static int tzdir_bss_count = -1;

  /* Force recomputation of cached values (Emacs).  */
  if (tzdir_bss_count != __bss_count)
  {
    tzdir_bss_count = __bss_count;
    dir[0] = 0;
  }
  if (dir[0] == 0)
  {
    if ((cp = getenv("TZDIR")))
    {
      strcpy(dir, cp);
    }
    else if ((cp = getenv("DJDIR")))
    {
      strcpy(dir, cp);
      strcat(dir, "/zoneinfo");
    }
    else
      strcpy(dir, "./");
  }
  return dir;
}

static int
tzload(const char *name, struct state *const sp, const int doextend)
{
  const char *p;
  int i;
  int fid;
  int nread = 0;
  char fullname[FILENAME_MAX + 1];
  const struct tzhead *tzhp;
  char buf[sizeof *tzhp + sizeof *sp + 2 * TZ_MAX_TIMES];
  int ttisgmtcnt, ttisstdcnt;

  sp->goback = sp->goahead = FALSE;
  if (name == NULL && (name = TZDEFAULT) == NULL)
    return -1;

  if (name[0] == ':')
    ++name;
  if (name[0] && !IS_ABSOLUTE(name))
  {
    if ((p = tzdir()) == NULL)
      return -1;
    if ((strlen(p) + strlen(name) + 1) >= sizeof fullname)
      return -1;
    strcpy(fullname, p);
    strcat(fullname, "/");
    strcat(fullname, name);
    name = fullname;
  }

  if ((fid = open(name, OPEN_MODE)) == -1)
  {
    const char *base = strrchr(name, '/');
    const char *bslash = strrchr(name, '\\');
    if (bslash && (!base || bslash > base))
      base = bslash;
    if (!base && name[1] == ':')
      base = name + 1;
    if (base)
      base++;
    else
      base = name;
    if (strcmp(base, "posixrules"))
      return -1;

    /* We've got a built-in copy of posixrules just in case */
    memcpy(buf, _posixrules_data, sizeof(_posixrules_data));
    i = sizeof(_posixrules_data);
  }
  else
  {
    nread = read(fid, buf, sizeof buf);
    if (close(fid) != 0 || nread < (int)sizeof *tzhp)
      return -1;
  }

  tzhp = (struct tzhead *) buf;
  ttisgmtcnt = (int) detzcode(tzhp->tzh_ttisgmtcnt);
  ttisstdcnt = (int) detzcode(tzhp->tzh_ttisstdcnt);
  sp->leapcnt = (int) detzcode(tzhp->tzh_leapcnt);
  sp->timecnt = (int) detzcode(tzhp->tzh_timecnt);
  sp->typecnt = (int) detzcode(tzhp->tzh_typecnt);
  sp->charcnt = (int) detzcode(tzhp->tzh_charcnt);
  if (sp->leapcnt < 0 || sp->leapcnt > TZ_MAX_LEAPS ||
      sp->typecnt <= 0 || sp->typecnt > TZ_MAX_TYPES ||
      sp->timecnt < 0 || sp->timecnt > TZ_MAX_TIMES ||
      sp->charcnt < 0 || sp->charcnt > TZ_MAX_CHARS ||
      (ttisstdcnt != sp->typecnt && ttisstdcnt != 0) ||
      (ttisgmtcnt != sp->typecnt && ttisgmtcnt != 0))
    return -1;
  if (nread < (int)(sizeof *tzhp +
      sp->timecnt * 4 +                        /* ats */
      sp->timecnt * sizeof(char) +             /* types */
      sp->typecnt * (4 + 2 * sizeof(char)) +   /* ttinfos */
      sp->charcnt * sizeof(char) +             /* chars */
      sp->leapcnt * 2 * 4 +                    /* lsinfos */
      ttisstdcnt * sizeof(char) +              /* ttisstds */
      ttisgmtcnt * sizeof(char)))              /* ttisgmts */
    return -1;
  p = buf + sizeof *tzhp;
  for (i = 0; i < sp->timecnt; ++i)
  {
    sp->ats[i] = detzcode(p);
    p += 4;
  }
  for (i = 0; i < sp->timecnt; ++i)
  {
    sp->types[i] = (unsigned char) *p++;
    if (sp->types[i] >= sp->typecnt)
      return -1;
  }
  for (i = 0; i < sp->typecnt; ++i)
  {
    register struct ttinfo *ttisp;

    ttisp = &sp->ttis[i];
    ttisp->tt_gmtoff = detzcode(p);
    p += 4;
    ttisp->tt_isdst = (unsigned char) *p++;
    if (ttisp->tt_isdst != 0 && ttisp->tt_isdst != 1)
      return -1;
    ttisp->tt_abbrind = (unsigned char) *p++;
    if (ttisp->tt_abbrind < 0 || ttisp->tt_abbrind > sp->charcnt)
      return -1;
  }
  for (i = 0; i < sp->charcnt; ++i)
    sp->chars[i] = *p++;
  sp->chars[i] = '\0';         /* ensure '\0' at end */
  for (i = 0; i < sp->leapcnt; ++i)
  {
    register struct lsinfo *lsisp;

    lsisp = &sp->lsis[i];
    lsisp->ls_trans = detzcode(p);
    p += 4;
    lsisp->ls_corr = detzcode(p);
    p += 4;
  }
  for (i = 0; i < sp->typecnt; ++i)
  {
    register struct ttinfo *ttisp;

    ttisp = &sp->ttis[i];
    if (ttisstdcnt == 0)
      ttisp->tt_ttisstd = FALSE;
    else
    {
      ttisp->tt_ttisstd = *p++;
      if (ttisp->tt_ttisstd != TRUE &&
          ttisp->tt_ttisstd != FALSE)
        return -1;
    }
  }
  for (i = 0; i < sp->typecnt; ++i)
  {
    register struct ttinfo *ttisp;

    ttisp = &sp->ttis[i];
    if (ttisgmtcnt == 0)
      ttisp->tt_ttisgmt = FALSE;
    else
    {
      ttisp->tt_ttisgmt = *p++;
      if (ttisp->tt_ttisgmt != TRUE &&
          ttisp->tt_ttisgmt != FALSE)
        return -1;
    }
  }
  /*
  ** Out-of-sort ats should mean we're running on a
  ** signed time_t system but using a data file with
  ** unsigned values (or vice versa).
  */
  for (i = 0; i < sp->timecnt; ++i)
    if ((i < sp->timecnt - 1 && sp->ats[i] > sp->ats[i + 1]) ||
  	(i == sp->timecnt - 1 && !TYPE_SIGNED(time_t) && sp->ats[i] > INT32_MAX))
    {
      if (TYPE_SIGNED(time_t))
      {
        /*
         ** Ignore the end (easy).
         */
        sp->timecnt = i + 1;
      }
      else
      {
        /*
        ** Ignore the beginning (harder).
        */
        register int j;

        /*
        ** Keep the record right before the
        ** epoch boundary,
        ** but tweak it so that it starts
        ** right with the epoch
        ** (thanks to Doug Bailey).
        */
        sp->ats[i] = 0;
        for (j = 0; j + i < sp->timecnt; ++j)
        {
          sp->ats[j] = sp->ats[j + i];
          sp->types[j] = sp->types[j + i];
        }
        sp->timecnt = j;
      }
      break;
    }

  /*
  ** DJGPP only supports old files (version 0).
  */
#if 0
  if (tzhp->tzh_version[0] != '\0')
    return -1;
#endif

  if (doextend && nread > 2 &&
      buf[0] == '\n' && buf[nread - 1] == '\n' &&
      sp->typecnt + 2 <= TZ_MAX_TYPES)
  {
    struct state ts;
    register int result;

    buf[nread - 1] = '\0';
    result = tzparse(&buf[1], &ts, FALSE);
    if (result == 0 && ts.typecnt == 2 && sp->charcnt + ts.charcnt <= TZ_MAX_CHARS)
    {
      for (i = 0; i < 2; ++i)
        ts.ttis[i].tt_abbrind += sp->charcnt;
      for (i = 0; i < ts.charcnt; ++i)
        sp->chars[sp->charcnt++] = ts.chars[i];
      i = 0;
      while (i < ts.timecnt && ts.ats[i] <= sp->ats[sp->timecnt - 1])
        ++i;
      while (i < ts.timecnt && sp->timecnt < TZ_MAX_TIMES)
      {
        sp->ats[sp->timecnt] = ts.ats[i];
        sp->types[sp->timecnt] = sp->typecnt + ts.types[i];
        ++sp->timecnt;
        ++i;
      }
      sp->ttis[sp->typecnt++] = ts.ttis[0];
      sp->ttis[sp->typecnt++] = ts.ttis[1];
    }
  }
  if (sp->timecnt > 1)
  {
    for (i = 1; i < sp->timecnt; ++i)
      if (typesequiv(sp, sp->types[i], sp->types[0]) &&
          differ_by_repeat(sp->ats[i], sp->ats[0]))
      {
        sp->goback = TRUE;
        break;
      }
    for (i = sp->timecnt - 2; i >= 0; --i)
      if (typesequiv(sp, sp->types[sp->timecnt - 1], sp->types[i]) &&
          differ_by_repeat(sp->ats[sp->timecnt - 1], sp->ats[i]))
      {
        sp->goahead = TRUE;
        break;
      }
  }
  /*
  ** If type 0 is unused in transitions,
  ** it's the type to use for early times.
  */
  for (i = 0; i < sp->typecnt; ++i)
    if (sp->types[i] == 0)
      break;
  i = (i >= sp->typecnt) ? 0 : -1;
  /*
  ** Absent the above,
  ** if there are transition times
  ** and the first transition is to a daylight time
  ** find the standard type less than and closest to
  ** the type of the first transition.
  */
  if (i < 0 && sp->timecnt > 0 && sp->ttis[sp->types[0]].tt_isdst)
  {
    i = sp->types[0];
    while (--i > -1)
      if (!sp->ttis[i].tt_isdst)
        break;
  }
  /*
  ** If no result yet, find the first standard type.
  ** If there is none, punt to type zero.
  */
  if (i < 0)
  {
    i = 0;
    while (sp->ttis[i].tt_isdst)
      if (++i >= sp->typecnt)
      {
        i = 0;
        break;
      }
  }
  sp->defaulttype = i;
  return 0;
}

static int
typesequiv(const struct state *const sp, const int a, const int b)
{
  register int	result;

  if (sp == NULL || a < 0 || a >= sp->typecnt || b < 0 || b >= sp->typecnt)
    result = FALSE;
  else
  {
    register const struct ttinfo *ap = &sp->ttis[a];
    register const struct ttinfo *bp = &sp->ttis[b];
    result = ap->tt_gmtoff == bp->tt_gmtoff &&
             ap->tt_isdst == bp->tt_isdst &&
             ap->tt_ttisstd == bp->tt_ttisstd &&
             ap->tt_ttisgmt == bp->tt_ttisgmt &&
             strcmp(&sp->chars[ap->tt_abbrind], &sp->chars[bp->tt_abbrind]) == 0;
  }
  return result;
}

static const int mon_lengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static const int year_lengths[2] = {
  DAYSPERNYEAR, DAYSPERLYEAR
};

/*
** Given a pointer into a time zone string, scan until a character that is not
** a valid character in a zone name is found.  Return a pointer to that
** character.
*/

static const char *
getzname(const char *strp)
{
  unsigned char c;

  while ((c = *strp) != '\0' && !is_digit(c) && c != ',' && c != '-' && c != '+')
    ++strp;
  return strp;
}

/*
** Given a pointer into an extended time zone string, scan until the ending
** delimiter of the zone name is located. Return a pointer to the delimiter.
**
** As with getzname above, the legal character set is actually quite
** restricted, with other characters producing undefined results.
** We don't do any checking here; checking is done later in common-case code.
*/

static const char *
getqzname(const char *strp, const int delim)
{
  register int c;

  while ((c = *strp) != '\0' && c != delim)
    ++strp;
  return strp;
}

/*
** Given a pointer into a time zone string, extract a number from that string.
** Check that the number is within a specified range; if it is not, return
** NULL.
** Otherwise, return a pointer to the first character not part of the number.
*/

static const char *
getnum(const char *strp, int *const nump, const int min, const int max)
{
  unsigned char c;
  int num;

  if (strp == NULL || !is_digit((unsigned char)*strp))
    return NULL;
  num = 0;
  while ((c = *strp) != '\0' && is_digit(c))
  {
    num = num * 10 + (c - '0');
    if (num > max)
      return NULL;  /* illegal value */
    ++strp;
  }
  if (num < min)
    return NULL;  /* illegal value */
  *nump = num;
  return strp;
}

/*
** Given a pointer into a time zone string, extract a number of seconds,
** in hh[:mm[:ss]] form, from the string.
** If any error occurs, return NULL.
** Otherwise, return a pointer to the first character not part of the number
** of seconds.
*/

static const char *
getsecs(const char *strp, int_fast32_t *const secsp)
{
  int num;

  /*
  ** `HOURSPERDAY * DAYSPERWEEK - 1' allows quasi-Posix rules like
  ** "M10.4.6/26", which does not conform to Posix,
  ** but which specifies the equivalent of
  ** ``02:00 on the first Sunday on or after 23 Oct''.
  */
  strp = getnum(strp, &num, 0, HOURSPERDAY * DAYSPERWEEK - 1);
  if (strp == NULL)
    return NULL;
  *secsp = num * (int_fast32_t)SECSPERHOUR;
  if (*strp == ':')
  {
    ++strp;
    strp = getnum(strp, &num, 0, MINSPERHOUR - 1);
    if (strp == NULL)
      return NULL;
    *secsp += num * SECSPERMIN;
    if (*strp == ':')
    {
      ++strp;
      /* `SECSPERMIN' allows for leap seconds. */
      strp = getnum(strp, &num, 0, SECSPERMIN);
      if (strp == NULL)
        return NULL;
      *secsp += num;
    }
  }
  return strp;
}

/*
** Given a pointer into a time zone string, extract an offset, in
** [+-]hh[:mm[:ss]] form, from the string.
** If any error occurs, return NULL.
** Otherwise, return a pointer to the first character not part of the time.
*/

static const char *
getoffset(const char *strp, int_fast32_t *const offsetp)
{
  int neg = 0;

  if (*strp == '-')
  {
    neg = 1;
    ++strp;
  }
  else if (*strp == '+')
    ++strp;
  strp = getsecs(strp, offsetp);
  if (strp == NULL)
    return NULL; /* illegal time */
  if (neg)
    *offsetp = -*offsetp;
  return strp;
}

/*
** Given a pointer into a time zone string, extract a rule in the form
** date[/time].  See POSIX section 8 for the format of "date" and "time".
** If a valid rule is not found, return NULL.
** Otherwise, return a pointer to the first character not part of the rule.
*/

static const char *
getrule(const char *strp, struct rule *const rulep)
{
  if (*strp == 'J')
  {
    /*
     ** Julian day.
     */
    rulep->r_type = JULIAN_DAY;
    ++strp;
    strp = getnum(strp, &rulep->r_day, 1, DAYSPERNYEAR);
  }
  else if (*strp == 'M')
  {
    /*
     ** Month, week, day.
     */
    rulep->r_type = MONTH_NTH_DAY_OF_WEEK;
    ++strp;
    strp = getnum(strp, &rulep->r_mon, 1, MONSPERYEAR);
    if (strp == NULL)
      return NULL;
    if (*strp++ != '.')
      return NULL;
    strp = getnum(strp, &rulep->r_week, 1, 5);
    if (strp == NULL)
      return NULL;
    if (*strp++ != '.')
      return NULL;
    strp = getnum(strp, &rulep->r_day, 0, DAYSPERWEEK - 1);
  }
  else if (is_digit((unsigned char)*strp))
  {
    /*
     ** Day of year.
     */
    rulep->r_type = DAY_OF_YEAR;
    strp = getnum(strp, &rulep->r_day, 0, DAYSPERLYEAR - 1);
  }
  else
    return NULL;                      /* invalid format */
  if (strp == NULL)
    return NULL;
  if (*strp == '/')
  {
    /*
     ** Time specified.
     */
    ++strp;
    strp = getsecs(strp, &rulep->r_time);
  }
  else
    rulep->r_time = 2 * SECSPERHOUR;  /* default = 2:00:00 */
  return strp;
}

/*
** Given the Epoch-relative time of January 1, 00:00:00 UTC, in a year, the
** year, a rule, and the offset from UTC at the time that rule takes effect,
** calculate the Epoch-relative time that rule takes effect.
*/

static time_t
transtime(const time_t janfirst, const int year, const struct rule *const rulep, const int_fast32_t offset)
{
  int leapyear;
  time_t value;
  int i;
  int d, m1, yy0, yy1, yy2, dow;

  INITIALIZE(value);
  leapyear = isleap(year);
  switch (rulep->r_type)
  {

  case JULIAN_DAY:
    /*
     ** Jn - Julian day, 1 == January 1, 60 == March 1 even in leap
     ** years.
     ** In non-leap years, or if the day number is 59 or less, just
     ** add SECSPERDAY times the day number-1 to the time of
     ** January 1, midnight, to get the day.
     */
    value = janfirst + (rulep->r_day - 1) * SECSPERDAY;
    if (leapyear && rulep->r_day >= 60)
      value += SECSPERDAY;
    break;

  case DAY_OF_YEAR:
    /*
     ** n - day of year.
     ** Just add SECSPERDAY times the day number to the time of
     ** January 1, midnight, to get the day.
     */
    value = janfirst + rulep->r_day * SECSPERDAY;
    break;

  case MONTH_NTH_DAY_OF_WEEK:
    /*
     ** Mm.n.d - nth "dth day" of month m.
     */
    value = janfirst;
    for (i = 0; i < rulep->r_mon - 1; ++i)
      value += mon_lengths[leapyear][i] * SECSPERDAY;

    /*
     ** Use Zeller's Congruence to get day-of-week of first day of
     ** month.
     */
    m1 = (rulep->r_mon + 9) % 12 + 1;
    yy0 = (rulep->r_mon <= 2) ? (year - 1) : year;
    yy1 = yy0 / 100;
    yy2 = yy0 % 100;
    dow = ((26 * m1 - 2) / 10 + 1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
    if (dow < 0)
      dow += DAYSPERWEEK;

    /*
     ** "dow" is the day-of-week of the first day of the month.  Get
     ** the day-of-month (zero-origin) of the first "dow" day of the
     ** month.
     */
    d = rulep->r_day - dow;
    if (d < 0)
      d += DAYSPERWEEK;
    for (i = 1; i < rulep->r_week; ++i)
    {
      if (d + DAYSPERWEEK >= mon_lengths[leapyear][rulep->r_mon - 1])
        break;
      d += DAYSPERWEEK;
    }

    /*
     ** "d" is the day-of-month (zero-origin) of the day we want.
     */
    value += d * SECSPERDAY;
    break;
  }

  /*
   ** "value" is the Epoch-relative time of 00:00:00 UTC on the day in
   ** question.  To get the Epoch-relative time of the specified local
   ** time on that day, add the transition time and the current offset
   ** from UTC.
   */
  return value + rulep->r_time + offset;
}

/*
** Given a POSIX section 8-style TZ string, fill in the rule tables as
** appropriate.
*/

static int
tzparse(const char *name, struct state *const sp, const int lastditch)
{
  const char *stdname;
  const char *dstname;
  size_t stdlen;
  size_t dstlen;
  int_fast32_t stdoffset;
  int_fast32_t dstoffset;
  time_t *atp;
  unsigned char *typep;
  char *cp;
  int load_result;
  static struct ttinfo zttinfo;

  INITIALIZE(dstname);
  stdname = name;
  if (lastditch)
  {
    stdlen = strlen(name);     /* length of standard zone name */
    name += stdlen;
    if (stdlen >= sizeof sp->chars)
      stdlen = (sizeof sp->chars) - 1;
  }
  else
  {
    if (*name == '<')
    {
      name++;
      stdname = name;
      name = getqzname(name, '>');
      if (*name != '>')
        return -1;
      stdlen = name - stdname;
      name++;
    }
    else
    {
      name = getzname(name);
      stdlen = name - stdname;
    }
    if (*name == '\0')
      return -1;
    name = getoffset(name, &stdoffset);
    if (name == NULL)
      return -1;
  }
  load_result = tzload(TZDEFRULES, sp, FALSE);
  if (load_result != 0)
    sp->leapcnt = 0;             /* so, we're off a little */
  if (*name != '\0')
  {
    if (*name == '<')
    {
      dstname = ++name;
      name = getqzname(name, '>');
      if (*name != '>')
        return -1;
      dstlen = name - dstname;
      name++;
    }
    else
    {
      dstname = name;
      name = getzname(name);
      dstlen = name - dstname;   /* length of DST zone name */
    }
    if (*name != '\0' && *name != ',' && *name != ';')
    {
      name = getoffset(name, &dstoffset);
      if (name == NULL)
        return -1;
    }
    else
      dstoffset = stdoffset - SECSPERHOUR;
    if (*name == '\0' && load_result != 0)
      name = TZDEFRULESTRING;
    if (*name == ',' || *name == ';')
    {
      struct rule start;
      struct rule end;
      int year;
      time_t janfirst;
      time_t starttime;
      time_t endtime;

      ++name;
      if ((name = getrule(name, &start)) == NULL)
        return -1;
      if (*name++ != ',')
        return -1;
      if ((name = getrule(name, &end)) == NULL)
        return -1;
      if (*name != '\0')
        return -1;
      sp->typecnt = 2;           /* standard time and DST */
      /*
      ** Two transitions per year, from EPOCH_YEAR forward.
      */
      sp->ttis[0] = sp->ttis[1] = zttinfo;
      sp->ttis[0].tt_gmtoff = -dstoffset;
      sp->ttis[0].tt_isdst = 1;
      sp->ttis[0].tt_abbrind = stdlen + 1;
      sp->ttis[1].tt_gmtoff = -stdoffset;
      sp->ttis[1].tt_isdst = 0;
      sp->ttis[1].tt_abbrind = 0;
      atp = sp->ats;
      typep = sp->types;
      janfirst = 0;
      sp->timecnt = 0;
      for (year = EPOCH_YEAR; sp->timecnt + 2 <= TZ_MAX_TIMES; ++year)
      {
        time_t newfirst;
        starttime = transtime(janfirst, year, &start, stdoffset);
        endtime = transtime(janfirst, year, &end, dstoffset);
        if (starttime > endtime)
        {
          *atp++ = endtime;
          *typep++ = 1;          /* DST ends */
          *atp++ = starttime;
          *typep++ = 0;          /* DST begins */
        }
        else
        {
          *atp++ = starttime;
          *typep++ = 0;          /* DST begins */
          *atp++ = endtime;
          *typep++ = 1;          /* DST ends */
        }
        sp->timecnt += 2;
        newfirst = janfirst;
        newfirst += year_lengths[isleap(year)] * SECSPERDAY;
        if (newfirst <= janfirst)
          break;
        janfirst = newfirst;
      }
    }
    else
    {
      register int_fast32_t theirstdoffset;
      register int_fast32_t theirdstoffset;
      register int_fast32_t theiroffset;
      int isdst;
      int i, j;

      if (*name != '\0')
        return -1;
      /*
      ** Initial values of theirstdoffset and theirdstoffset.
      */
      theirstdoffset = 0;
      for (i = 0; i < sp->timecnt; ++i)
      {
        j = sp->types[i];
        if (!sp->ttis[j].tt_isdst)
        {
          theirstdoffset = -sp->ttis[j].tt_gmtoff;
          break;
        }
      }
      theirdstoffset = 0;
      for (i = 0; i < sp->timecnt; ++i)
      {
        j = sp->types[i];
        if (sp->ttis[j].tt_isdst)
        {
          theirdstoffset = -sp->ttis[j].tt_gmtoff;
          break;
        }
      }
      /*
      ** Initially we're assumed to be in standard time.
      */
      isdst = FALSE;
      theiroffset = theirstdoffset;
      /*
      ** Now juggle transition times and types
      ** tracking offsets as you do.
      */
      for (i = 0; i < sp->timecnt; ++i)
      {
        j = sp->types[i];
        sp->types[i] = sp->ttis[j].tt_isdst;
        if (sp->ttis[j].tt_ttisgmt)
        {
          /* No adjustment to transition time */
        }
        else
        {
          /*
           ** If summer time is in effect, and the
           ** transition time was not specified as
           ** standard time, add the summer time
           ** offset to the transition time;
           ** otherwise, add the standard time
           ** offset to the transition time.
           */
           /*
            ** Transitions from DST to DDST
            ** will effectively disappear since
            ** POSIX provides for only one DST
            ** offset.
            */
          if (isdst && !sp->ttis[j].tt_ttisstd)
            sp->ats[i] += dstoffset - theirdstoffset;
          else
            sp->ats[i] += stdoffset - theirstdoffset;
        }
        theiroffset = -sp->ttis[j].tt_gmtoff;
        if (sp->ttis[j].tt_isdst)
          theirdstoffset = theiroffset;
        else
          theirstdoffset = theiroffset;
      }
      /*
      ** Finally, fill in ttis.
      */
      sp->ttis[0] = sp->ttis[1] = zttinfo;
      sp->ttis[0].tt_gmtoff = -stdoffset;
      sp->ttis[0].tt_isdst = FALSE;
      sp->ttis[0].tt_abbrind = 0;
      sp->ttis[1].tt_gmtoff = -dstoffset;
      sp->ttis[1].tt_isdst = TRUE;
      sp->ttis[1].tt_abbrind = stdlen + 1;
      sp->typecnt = 2;
    }
  }
  else
  {
    dstlen = 0;
    sp->typecnt = 1;             /* only standard time */
    sp->timecnt = 0;
    sp->ttis[0].tt_gmtoff = -stdoffset;
    sp->ttis[0].tt_isdst = 0;
    sp->ttis[0].tt_abbrind = 0;
  }
  sp->charcnt = stdlen + 1;
  if (dstlen != 0)
    sp->charcnt += dstlen + 1;
  if (sp->charcnt > (int)sizeof sp->chars)
    return -1;
  cp = sp->chars;
  (void) strncpy(cp, stdname, stdlen);
  cp += stdlen;
  *cp++ = '\0';
  if (dstlen != 0)
  {
    (void) strncpy(cp, dstname, dstlen);
    *(cp + dstlen) = '\0';
  }
  return 0;
}

static void
gmtload(struct state *const sp)
{
  if (tzload(gmt, sp, TRUE) != 0)
    (void) tzparse(gmt, sp, TRUE);
}

void
tzsetwall(void)
{
  if (lcl_is_set == -1)
    return;
  lcl_is_set = -1;
  if (tzload((char *) NULL, lclptr, TRUE) != 0)
    gmtload(lclptr);
  settzname();
}

void
tzset(void)
{
  const char * name;
  static unsigned last_env_changed = 0;

  /* If environ didn't changed since last time, don't waste time
     looking at $TZ.  */
  if (lcl_is_set > 0 && __environ_changed == last_env_changed)
    return;

  /* If environ did change, but $TZ wasn't changed since last time we
     were called, we are all done here.  */
  last_env_changed = __environ_changed;
  name = getenv("TZ");
  /* Use stricmp, since if TZ points to a file name, we need to be
     case-insensitive.  */
  if (lcl_is_set > 0 && (name == NULL || stricmp(name, lcl_TZname) == 0))
    return;

  /* On to some *real* work... */
  if (name == NULL)
  {
    tzsetwall();
    lcl_is_set = 1;
    return;
  }
  lcl_is_set = strlen(name) < sizeof lcl_TZname;
  if (lcl_is_set)
    strcpy(lcl_TZname, name);

  if (*name == '\0')
  {
    /*
     ** User wants it fast rather than right.
     */
    lclptr->leapcnt = 0;       /* so, we're off a little */
    lclptr->timecnt = 0;
    lclptr->ttis[0].tt_isdst = 0;
    lclptr->ttis[0].tt_gmtoff = 0;
    lclptr->ttis[0].tt_abbrind = 0;
    (void) strcpy(lclptr->chars, gmt);
  }
  else if (tzload(name, lclptr, TRUE) != 0)
    if (name[0] == ':' || tzparse(name, lclptr, FALSE) != 0)
      gmtload(lclptr);
  settzname();
}

/*
** The easy way to behave "as if no library function calls" localtime
** is to not call it--so we drop its guts into "localsub", which can be
** freely called.  (And no, the PANS doesn't require the above behavior--
** but it *is* desirable.)
**
** The unused offset argument is for the benefit of mktime variants.
*/

/*ARGSUSED*/
static struct tm *
localsub(const time_t *const timep, const int_fast32_t offset, struct tm *const tmp)
{
  const struct state *sp;
  const struct ttinfo *ttisp;
  int i;
  struct tm *result;
  const time_t t = *timep;

  sp = lclptr;
  if ((sp->goback && t < sp->ats[0]) ||
      (sp->goahead && t > sp->ats[sp->timecnt - 1]))
  {
    time_t newt = t;
    register time_t seconds;
    register time_t tcycles;
    register int_fast64_t icycles;

    if (t < sp->ats[0])
      seconds = sp->ats[0] - t;
    else
      seconds = t - sp->ats[sp->timecnt - 1];
    --seconds;
    tcycles = seconds / YEARSPERREPEAT / AVGSECSPERYEAR;
    ++tcycles;
    icycles = tcycles;
    if (tcycles - icycles >= 1 || icycles - tcycles >= 1)
      return NULL;
    seconds = icycles;
    seconds *= YEARSPERREPEAT;
    seconds *= AVGSECSPERYEAR;
    if (t < sp->ats[0])
      newt += seconds;
    else
      newt -= seconds;
    if (newt < sp->ats[0] || newt > sp->ats[sp->timecnt - 1])
      return NULL;        /* "cannot happen" */
    result = localsub(&newt, offset, tmp);
    if (result == tmp)
    {
      register time_t newy;

      newy = tmp->tm_year;
      if (t < sp->ats[0])
        newy -= icycles * YEARSPERREPEAT;
      else
        newy += icycles * YEARSPERREPEAT;
      tmp->tm_year = newy;
      if (tmp->tm_year != (int)newy)
        return NULL;
    }
    return result;
  }
  if (sp->timecnt == 0 || t < sp->ats[0])
    i = sp->defaulttype;
  else
  {
    register int lo = 1;
    register int hi = sp->timecnt;

    while (lo < hi)
    {
      register int mid = (lo + hi) >> 1;

      if (t < sp->ats[mid])
        hi = mid;
      else
        lo = mid + 1;
    }
    i = (int) sp->types[lo - 1];
  }
  ttisp = &sp->ttis[i];
  /*
   ** To get (wrong) behavior that's compatible with System V Release 2.0
   ** you'd replace the statement below with
   ** t += ttisp->tt_gmtoff;
   ** timesub(&t, 0L, sp, tmp);
   */
  result = timesub(&t, ttisp->tt_gmtoff, sp, tmp);
  tmp->tm_isdst = ttisp->tt_isdst;
  tzname[tmp->tm_isdst] = unconst(&sp->chars[ttisp->tt_abbrind], char *);
  tmp->tm_zone = unconst(&sp->chars[ttisp->tt_abbrind], char *);
  return result;
}

struct tm *
localtime(const time_t *const timep)
{
  static struct tm tm;

  tzset();
  return localtime_r(timep, &tm);
}

struct tm *
localtime_r( const time_t * __restrict__ timep, struct tm * __restrict__ tm)
{
  return localsub(timep, 0L, tm);;
}

/*
** gmtsub is to gmtime as localsub is to localtime.
*/

static struct tm *
gmtsub(const time_t *const timep, const int_fast32_t offset, struct tm *const tmp)
{
  register struct tm *result;

  if (!gmt_is_set)
  {
    gmt_is_set = TRUE;
    gmtload(gmtptr);
  }
  result = timesub(timep, offset, gmtptr, tmp);
  /*
   ** Could get fancy here and deliver something such as
   ** "UTC+xxxx" or "UTC-xxxx" if offset is non-zero,
   ** but this is no time for a treasure hunt.
   */
  if (offset != 0)
    tmp->tm_zone = wildabbr;
  else
    tmp->tm_zone = gmtptr->chars;
  return result;
}

struct tm *
gmtime(const time_t *const timep)
{
  static struct tm tm;

  return gmtime_r(timep, &tm);
}

struct tm *
gmtime_r(const time_t * __restrict__ timep, struct tm * __restrict__ tm)
{
  return gmtsub(timep, 0L, tm);
}

/*
** Return the number of leap years through the end of the given year
** where, to make the math easy, the answer for year zero is defined as zero.
*/

static int
leaps_thru_end_of(register const int y)
{
  return (y >= 0) ? (y / 4 - y / 100 + y / 400) : -(leaps_thru_end_of(-(y + 1)) + 1);
}

static struct tm *
timesub(const time_t *const timep, const int_fast32_t offset, register const struct state *const sp, register struct tm *const tmp)
{
  const struct lsinfo *lp;
  time_t tdays;
  int idays;     /* unsigned would be so 2003 */
  int_fast64_t rem;
  int y;
  const int *ip;
  int_fast64_t corr;
  int hit;
  int i;

  corr = 0;
  hit = 0;
  i = sp->leapcnt;
  while (--i > -1)
  {
    lp = &sp->lsis[i];
    if (*timep >= lp->ls_trans)
    {
      if (*timep == lp->ls_trans)
      {
        hit = ((i == 0 && lp->ls_corr > 0) || lp->ls_corr > sp->lsis[i - 1].ls_corr);
        if (hit)
          while (i > 0 &&
                 sp->lsis[i].ls_trans == sp->lsis[i - 1].ls_trans + 1 &&
                 sp->lsis[i].ls_corr == sp->lsis[i - 1].ls_corr + 1)
          {
            ++hit;
            --i;
          }
      }
      corr = lp->ls_corr;
      break;
    }
  }
  y = EPOCH_YEAR;
  tdays = *timep / SECSPERDAY;
  rem = *timep - tdays * SECSPERDAY;
  while (tdays < 0 || tdays >= (time_t)year_lengths[isleap(y)])
  {
    int newy;
    register time_t tdelta;
    register int idelta;
    register int leapdays;

    tdelta = tdays / DAYSPERLYEAR;
    idelta = tdelta;
    if (tdelta - idelta >= 1 || idelta - tdelta >= 1)
      return NULL;
    if (idelta == 0)
      idelta = (tdays < 0) ? -1 : 1;
    newy = y;
    if (increment_overflow(&newy, idelta))
      return NULL;
    leapdays = leaps_thru_end_of(newy - 1) - leaps_thru_end_of(y - 1);
    tdays -= ((time_t) newy - y) * DAYSPERNYEAR;
    tdays -= leapdays;
    y = newy;
  }
  {
    register int_fast32_t seconds;
    register time_t half_second = 0.5;

    seconds = tdays * SECSPERDAY + half_second;
    tdays = seconds / SECSPERDAY;
    rem += seconds - tdays * SECSPERDAY;
  }
  /*
  ** Given the range, we can now fearlessly cast...
  */
  idays = tdays;
  rem += offset - corr;
  while (rem < 0)
  {
    rem += SECSPERDAY;
    --idays;
  }
  while (rem >= SECSPERDAY)
  {
    rem -= SECSPERDAY;
    ++idays;
  }
  while (idays < 0)
  {
    if (increment_overflow(&y, -1))
      return NULL;
    idays += year_lengths[isleap(y)];
  }
  while (idays >= year_lengths[isleap(y)])
  {
    idays -= year_lengths[isleap(y)];
    if (increment_overflow(&y, 1))
      return NULL;
  }
  tmp->tm_year = y;
  if (increment_overflow(&tmp->tm_year, -TM_YEAR_BASE))
    return NULL;
  tmp->tm_yday = idays;
  /*
  ** The "extra" mods below avoid overflow problems.
  */
  tmp->tm_wday = EPOCH_WDAY +
                 ((y - EPOCH_YEAR) % DAYSPERWEEK) *
                 (DAYSPERNYEAR % DAYSPERWEEK) +
                 leaps_thru_end_of(y - 1) -
                 leaps_thru_end_of(EPOCH_YEAR - 1) +
                 idays;
  tmp->tm_wday %= DAYSPERWEEK;
  if (tmp->tm_wday < 0)
    tmp->tm_wday += DAYSPERWEEK;
  tmp->tm_hour = (int) (rem / SECSPERHOUR);
  rem %= SECSPERHOUR;
  tmp->tm_min = (int) (rem / SECSPERMIN);
  /*
  ** A positive leap second requires a special
  ** representation. This uses "... ??:59:60" et seq.
  */
  tmp->tm_sec = (int) (rem % SECSPERMIN) + hit;
  ip = mon_lengths[isleap(y)];
  for (tmp->tm_mon = 0; idays >= ip[tmp->tm_mon]; ++(tmp->tm_mon))
    idays -= ip[tmp->tm_mon];
  tmp->tm_mday = (int) (idays + 1);
  tmp->tm_isdst = 0;
  tmp->tm_gmtoff = offset;
  return tmp;
}

/*
** A la X3J11
*/

char *
asctime_r(const struct tm * __restrict__ timeptr, char * __restrict__ result)
{
  static const char wday_name[DAYSPERWEEK][3] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char mon_name[MONSPERYEAR][3] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  (void) sprintf(result, "%.3s %.3s%3d %02d:%02d:%02d %d\n",
                 wday_name[timeptr->tm_wday],
                 mon_name[timeptr->tm_mon],
                 timeptr->tm_mday, timeptr->tm_hour,
                 timeptr->tm_min, timeptr->tm_sec,
                 TM_YEAR_BASE + timeptr->tm_year);
  return result;
}

char *
asctime(const struct tm *timeptr)
{
  static char result[26];

  return asctime_r( timeptr, result);
}

char *
ctime_r(const time_t *timep, char *result)
{
  struct tm tm;

  return asctime_r(localtime_r(timep, &tm), result);
}

char *
ctime(const time_t *const timep)
{
  return asctime(localtime(timep));
}

/*
** Adapted from code provided by Robert Elz, who writes:
**      The "best" way to do mktime I think is based on an idea of Bob
**      Kridle's (so its said...) from a long time ago. (mtxinu!kridle now).
**      It does a binary search of the time_t space.  Since time_t's are
**      just 32 bits, its a max of 32 iterations (even at 64 bits it
**      would still be very reasonable).
*/

#ifndef WRONG
#define WRONG   (-1)
#endif /* !defined WRONG */

/*
** Normalize logic courtesy Paul Eggert.
*/

static int
increment_overflow(int *const ip, int j)
{
  register int const i = *ip;

  /*
  ** If i >= 0 there can only be overflow if i + j > INT_MAX
  ** or if j > INT_MAX - i; given i >= 0, INT_MAX - i cannot overflow.
  ** If i < 0 there can only be overflow if i + j < INT_MIN
  ** or if j < INT_MIN - i; given i < 0, INT_MIN - i cannot overflow.
  */
  if ((i >= 0) ? (j > INT_MAX - i) : (j < INT_MIN - i))
    return TRUE;
  *ip += j;
  return FALSE;
}

static int
increment_overflow32(int_fast32_t *const lp, int const m)
{
  register int_fast32_t const l = *lp;

  if ((l >= 0) ? (m > INT_FAST32_MAX - l) : (m < INT_FAST32_MIN - l))
    return TRUE;
  *lp += m;
  return FALSE;
}

static int
normalize_overflow(int *const tensptr, int *const unitsptr, const int base)
{
  register int tensdelta;

  tensdelta = (*unitsptr >= 0) ? (*unitsptr / base) : (-1 - (-1 - *unitsptr) / base);
  *unitsptr -= tensdelta * base;
  return increment_overflow(tensptr, tensdelta);
}

static int
normalize_overflow32(int_fast32_t *const tensptr, int *const unitsptr, const int base)
{
  register int tensdelta;

  tensdelta = (*unitsptr >= 0) ? (*unitsptr / base) : (-1 - (-1 - *unitsptr) / base);
  *unitsptr -= tensdelta * base;
  return increment_overflow32(tensptr, tensdelta);
}

static int
tmcomp(const struct tm *const atmp, const struct tm *const btmp)
{
  int result;

  if ((result = (atmp->tm_year - btmp->tm_year)) == 0 &&
      (result = (atmp->tm_mon - btmp->tm_mon)) == 0 &&
      (result = (atmp->tm_mday - btmp->tm_mday)) == 0 &&
      (result = (atmp->tm_hour - btmp->tm_hour)) == 0 &&
      (result = (atmp->tm_min - btmp->tm_min)) == 0)
    result = atmp->tm_sec - btmp->tm_sec;
  return result;
}

static time_t
time2sub(struct tm *const tmp, struct tm *(*const funcp)(const time_t *, int_fast32_t, struct tm *), const int_fast32_t offset, int *const okayp, const int do_norm_secs)
{
  register const struct state *sp;
  register int dir;
  register int i, j;
  register int saved_seconds;
  register int_fast32_t li;
  register time_t lo;
  register time_t hi;
  int_fast32_t y;
  time_t newt;
  time_t t;
  struct tm yourtm, mytm;

  *okayp = FALSE;
  yourtm = *tmp;
  if (do_norm_secs)
  {
    if (normalize_overflow(&yourtm.tm_min, &yourtm.tm_sec, SECSPERMIN))
      return WRONG;
  }
  if (normalize_overflow(&yourtm.tm_hour, &yourtm.tm_min, MINSPERHOUR))
    return WRONG;
  if (normalize_overflow(&yourtm.tm_mday, &yourtm.tm_hour, HOURSPERDAY))
    return WRONG;
  y = yourtm.tm_year;
  if (normalize_overflow32(&y, &yourtm.tm_mon, MONSPERYEAR))
    return WRONG;
  /*
  ** Turn y into an actual year number for now.
  ** It is converted back to an offset from TM_YEAR_BASE later.
  */
  if (increment_overflow32(&y, TM_YEAR_BASE))
    return WRONG;
  while (yourtm.tm_mday <= 0)
  {
    if (increment_overflow32(&y, -1))
      return WRONG;
    li = y + (1 < yourtm.tm_mon);
    yourtm.tm_mday += year_lengths[isleap(li)];
  }
  while (yourtm.tm_mday > DAYSPERLYEAR)
  {
    li = y + (1 < yourtm.tm_mon);
    yourtm.tm_mday -= year_lengths[isleap(li)];
    if (increment_overflow32(&y, 1))
      return WRONG;
  }
  for ( ; ; )
  {
    i = mon_lengths[isleap(y)][yourtm.tm_mon];
    if (yourtm.tm_mday <= i)
      break;
    yourtm.tm_mday -= i;
    if (++yourtm.tm_mon >= MONSPERYEAR)
    {
      yourtm.tm_mon = 0;
      if (increment_overflow32(&y, 1))
        return WRONG;
    }
  }
  if (increment_overflow32(&y, -TM_YEAR_BASE))
    return WRONG;
  yourtm.tm_year = y;
  if (yourtm.tm_year != y)
    return WRONG;
  if (yourtm.tm_sec >= 0 && yourtm.tm_sec < SECSPERMIN)
    saved_seconds = 0;
  else if (y + TM_YEAR_BASE < EPOCH_YEAR)
  {
    /*
    ** We can't set tm_sec to 0, because that might push the
    ** time below the minimum representable time.
    ** Set tm_sec to 59 instead.
    ** This assumes that the minimum representable time is
    ** not in the same minute that a leap second was deleted from,
    ** which is a safer assumption than using 58 would be.
    */
    if (increment_overflow(&yourtm.tm_sec, 1 - SECSPERMIN))
      return WRONG;
    saved_seconds = yourtm.tm_sec;
    yourtm.tm_sec = SECSPERMIN - 1;
  }
  else
  {
    saved_seconds = yourtm.tm_sec;
    yourtm.tm_sec = 0;
  }
  /*
  ** Do a binary search (this works whatever time_t's type is).
  */
  if (!TYPE_SIGNED(time_t))
  {
    lo = 0;
    hi = lo - 1;
  }
  else if (!TYPE_INTEGRAL(time_t))
  {
    if (sizeof(time_t) > sizeof(float))
      hi = (time_t) DBL_MAX;
    else
      hi = (time_t) FLT_MAX;
    lo = -hi;
  }
  else
  {
    lo = 1;
    for (i = 0; i < (int) TYPE_BIT(time_t) - 1; ++i)
      lo *= 2;
    hi = -(lo + 1);
  }
  for ( ; ; )
  {
    t = lo / 2 + hi / 2;
    if (t < lo)
      t = lo;
    else if (t > hi)
      t = hi;
    if ((*funcp)(&t, offset, &mytm) == NULL)
    {
      /*
      ** Assume that t is too extreme to be represented in
      ** a struct tm; arrange things so that it is less
      ** extreme on the next pass.
      */
      dir = (t > 0) ? 1 : -1;
    }
    else
      dir = tmcomp(&mytm, &yourtm);
    if (dir != 0)
    {
      if (t == lo)
      {
        ++t;
        if (t <= lo)
          return WRONG;
        ++lo;
      }
      else if (t == hi)
      {
        --t;
        if (t >= hi)
          return WRONG;
        --hi;
      }
      if (lo > hi)
        return WRONG;
      if (dir > 0)
        hi = t;
      else
        lo = t;
      continue;
    }
    if (yourtm.tm_isdst < 0 || mytm.tm_isdst == yourtm.tm_isdst)
      break;
    /*
    ** Right time, wrong type.
    ** Hunt for right time, right type.
    ** It's okay to guess wrong since the guess
    ** gets checked.
    */
    sp = (const struct state *)((funcp == localsub) ? lclptr : gmtptr);
    for (i = sp->typecnt - 1; i >= 0; --i)
    {
      if (sp->ttis[i].tt_isdst != yourtm.tm_isdst)
        continue;
      for (j = sp->typecnt - 1; j >= 0; --j)
      {
        if (sp->ttis[j].tt_isdst == yourtm.tm_isdst)
          continue;
        newt = t + sp->ttis[j].tt_gmtoff - sp->ttis[i].tt_gmtoff;
        if ((*funcp)(&newt, offset, &mytm) == NULL)
          continue;
        if (tmcomp(&mytm, &yourtm) != 0)
          continue;
        if (mytm.tm_isdst != yourtm.tm_isdst)
          continue;
        /*
        ** We have a match.
        */
        t = newt;
        goto label;
      }
    }
    return WRONG;
  }
label:
  newt = t + saved_seconds;
  if ((newt < t) != (saved_seconds < 0))
    return WRONG;
  t = newt;
  if ((*funcp)(&t, offset, tmp))
    *okayp = TRUE;
  return t;
}

static time_t
time2(struct tm *const tmp, struct tm *(*const funcp)(const time_t *, int_fast32_t, struct tm *), const int_fast32_t offset, int *const okayp)
{
  time_t t;

  /*
  ** First try without normalization of seconds
  ** (in case tm_sec contains a value associated with a leap second).
  ** If that fails, try with normalization of seconds.
  */
  t = time2sub(tmp, funcp, offset, okayp, FALSE);
  return *okayp ? t : time2sub(tmp, funcp, offset, okayp, TRUE);
}

static time_t
time1(struct tm *const tmp, struct tm *(*const funcp)(const time_t *, int_fast32_t, struct tm *), const int_fast32_t offset)
{
  register time_t t;
  register const struct state *sp;
  register int samei, otheri;
  register int sameind, otherind;
  register int i;
  register int nseen;
  int seen[TZ_MAX_TYPES];
  int types[TZ_MAX_TYPES];
  int okay;

  if (tmp == NULL)
  {
    errno = EINVAL;
    return WRONG;
  }
  if (tmp->tm_isdst > 1)
    tmp->tm_isdst = 1;
  t = time2(tmp, funcp, offset, &okay);
  if (okay || tmp->tm_isdst < 0)
    return t;
  /*
  ** We're supposed to assume that somebody took a time of one type
  ** and did some math on it that yielded a "struct tm" that's bad.
  ** We try to divine the type they started from and adjust to the
  ** type they need.
  */
  sp = (const struct state *) ((funcp == localsub) ?  lclptr : gmtptr);
  for (i = 0; i < sp->typecnt; ++i)
    seen[i] = FALSE;
  nseen = 0;
  for (i = sp->timecnt - 1; i >= 0; --i)
    if (!seen[sp->types[i]])
    {
      seen[sp->types[i]] = TRUE;
      types[nseen++] = sp->types[i];
    }
  for (sameind = 0; sameind < nseen; ++sameind)
  {
    samei = types[sameind];
    if (sp->ttis[samei].tt_isdst != tmp->tm_isdst)
      continue;
    for (otherind = 0; otherind < nseen; ++otherind)
    {
      otheri = types[otherind];
      if (sp->ttis[otheri].tt_isdst == tmp->tm_isdst)
        continue;
      tmp->tm_sec += sp->ttis[otheri].tt_gmtoff - sp->ttis[samei].tt_gmtoff;
      tmp->tm_isdst = !tmp->tm_isdst;
      t = time2(tmp, funcp, offset, &okay);
      if (okay)
        return t;
      tmp->tm_sec -= sp->ttis[otheri].tt_gmtoff - sp->ttis[samei].tt_gmtoff;
      tmp->tm_isdst = !tmp->tm_isdst;
    }
  }
  return WRONG;
}

time_t
mktime(struct tm * tmp)
{
  tzset();
  return time1(tmp, localsub, 0L);
}
