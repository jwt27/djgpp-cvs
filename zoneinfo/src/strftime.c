#ifndef lint
#ifndef NOID
static char	elsieid[] = "@(#)strftime.c	7.48";
/*
** Based on the UCB version with the ID appearing below.
** This is ANSIish only when "multibyte character == plain character".
*/
#endif /* !defined NOID */
#endif /* !defined lint */

#include "private.h"

/*
** Copyright (c) 1989 The Regents of the University of California.
** All rights reserved.
**
** Redistribution and use in source and binary forms are permitted
** provided that the above copyright notice and this paragraph are
** duplicated in all such forms and that any documentation,
** advertising materials, and other materials related to such
** distribution and use acknowledge that the software was developed
** by the University of California, Berkeley.  The name of the
** University may not be used to endorse or promote products derived
** from this software without specific prior written permission.
** THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef LIBC_SCCS
#ifndef lint
static const char	sccsid[] = "@(#)strftime.c	5.4 (Berkeley) 3/14/89";
#endif /* !defined lint */
#endif /* !defined LIBC_SCCS */

#include "tzfile.h"
#include "fcntl.h"
#include "locale.h"

struct lc_time_T {
	const char *	mon[12];
	const char *	month[12];
	const char *	wday[7];
	const char *	weekday[7];
	const char *	X_fmt;
	const char *	x_fmt;
	const char *	c_fmt;
	const char *	am;
	const char *	pm;
	const char *	date_fmt;
};

#ifdef LOCALE_HOME
#include "sys/stat.h"
static struct lc_time_T		localebuf;
static struct lc_time_T *	_loc P((void));
#define Locale	_loc()
#endif /* defined LOCALE_HOME */
#ifndef LOCALE_HOME
#define Locale	(&C_time_locale)
#endif /* !defined LOCALE_HOME */

static const struct lc_time_T	C_time_locale = {
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	}, {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	}, {
		"Sun", "Mon", "Tue", "Wed",
		"Thu", "Fri", "Sat"
	}, {
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday"
	},

	/* X_fmt */
	"%H:%M:%S",

	/*
	** x_fmt
	** Since the C language standard calls for
	** "date, using locale's date format," anything goes.
	** Using just numbers (as here) makes Quakers happier;
	** it's also compatible with SVR4.
	*/
	"%m/%d/%y",

	/*
	** c_fmt
	** Note that
	**	"%a %b %d %H:%M:%S %Y"
	** is used by Solaris 2.3.
	*/
	"%D %X",	/* %m/%d/%y %H:%M:%S */

	/* am */
	"AM",

	/* pm */
	"PM",

	/* date_fmt */
	"%a %b %e %H:%M:%S %Z %Y"
};

static char *	_add P((const char *, char *, const char *));
static char *	_conv P((int, const char *, char *, const char *));
static char *	_fmt P((const char *, const struct tm *, char *, const char *));

size_t strftime P((char *, size_t, const char *, const struct tm *));

extern char *	tzname[];

size_t
strftime(s, maxsize, format, t)
char * const		s;
const size_t		maxsize;
const char * const	format;
const struct tm * const	t;
{
	char *	p;

	tzset();
#ifdef LOCALE_HOME
	localebuf.mon[0] = 0;
#endif /* defined LOCALE_HOME */
	p = _fmt(((format == NULL) ? "%c" : format), t, s, s + maxsize);
	if (p == s + maxsize)
		return 0;
	*p = '\0';
	return p - s;
}

static char *
_fmt(format, t, pt, ptlim)
const char *		format;
const struct tm * const	t;
char *			pt;
const char * const	ptlim;
{
	for ( ; *format; ++format) {
		if (*format == '%') {
label:
			switch (*++format) {
			case '\0':
				--format;
				break;
			case 'A':
				pt = _add((t->tm_wday < 0 || t->tm_wday > 6) ?
					"?" : Locale->weekday[t->tm_wday],
					pt, ptlim);
				continue;
			case 'a':
				pt = _add((t->tm_wday < 0 || t->tm_wday > 6) ?
					"?" : Locale->wday[t->tm_wday],
					pt, ptlim);
				continue;
			case 'B':
				pt = _add((t->tm_mon < 0 || t->tm_mon > 11) ?
					"?" : Locale->month[t->tm_mon],
					pt, ptlim);
				continue;
			case 'b':
			case 'h':
				pt = _add((t->tm_mon < 0 || t->tm_mon > 11) ?
					"?" : Locale->mon[t->tm_mon],
					pt, ptlim);
				continue;
			case 'C':
				/*
				** %C used to do a...
				**	_fmt("%a %b %e %X %Y", t);
				** ...whereas now POSIX 1003.2 calls for
				** something completely different.
				** (ado, 5/24/93)
				*/
				pt = _conv((t->tm_year + TM_YEAR_BASE) / 100,
					"%02d", pt, ptlim);
				continue;
			case 'c':
				pt = _fmt(Locale->c_fmt, t, pt, ptlim);
				continue;
			case 'D':
				pt = _fmt("%m/%d/%y", t, pt, ptlim);
				continue;
			case 'd':
				pt = _conv(t->tm_mday, "%02d", pt, ptlim);
				continue;
			case 'E':
			case 'O':
				/*
				** POSIX locale extensions, a la
				** Arnold Robbins' strftime version 3.0.
				** The sequences
				**	%Ec %EC %Ex %Ey %EY
				**	%Od %oe %OH %OI %Om %OM
				**	%OS %Ou %OU %OV %Ow %OW %Oy
				** are supposed to provide alternate
				** representations.
				** (ado, 5/24/93)
				*/
				goto label;
			case 'e':
				pt = _conv(t->tm_mday, "%2d", pt, ptlim);
				continue;
			case 'H':
				pt = _conv(t->tm_hour, "%02d", pt, ptlim);
				continue;
			case 'I':
				pt = _conv((t->tm_hour % 12) ?
					(t->tm_hour % 12) : 12,
					"%02d", pt, ptlim);
				continue;
			case 'j':
				pt = _conv(t->tm_yday + 1, "%03d", pt, ptlim);
				continue;
			case 'k':
				/*
				** This used to be...
				**	_conv(t->tm_hour % 12 ?
				**		t->tm_hour % 12 : 12, 2, ' ');
				** ...and has been changed to the below to
				** match SunOS 4.1.1 and Arnold Robbins'
				** strftime version 3.0.  That is, "%k" and
				** "%l" have been swapped.
				** (ado, 5/24/93)
				*/
				pt = _conv(t->tm_hour, "%2d", pt, ptlim);
				continue;
#ifdef KITCHEN_SINK
			case 'K':
				/*
				** After all this time, still unclaimed!
				*/
				pt = _add("kitchen sink", pt, ptlim);
				continue;
#endif /* defined KITCHEN_SINK */
			case 'l':
				/*
				** This used to be...
				**	_conv(t->tm_hour, 2, ' ');
				** ...and has been changed to the below to
				** match SunOS 4.1.1 and Arnold Robbin's
				** strftime version 3.0.  That is, "%k" and
				** "%l" have been swapped.
				** (ado, 5/24/93)
				*/
				pt = _conv((t->tm_hour % 12) ?
					(t->tm_hour % 12) : 12,
					"%2d", pt, ptlim);
				continue;
			case 'M':
				pt = _conv(t->tm_min, "%02d", pt, ptlim);
				continue;
			case 'm':
				pt = _conv(t->tm_mon + 1, "%02d", pt, ptlim);
				continue;
			case 'n':
				pt = _add("\n", pt, ptlim);
				continue;
			case 'p':
				pt = _add((t->tm_hour >= 12) ?
					Locale->pm :
					Locale->am,
					pt, ptlim);
				continue;
			case 'R':
				pt = _fmt("%H:%M", t, pt, ptlim);
				continue;
			case 'r':
				pt = _fmt("%I:%M:%S %p", t, pt, ptlim);
				continue;
			case 'S':
				pt = _conv(t->tm_sec, "%02d", pt, ptlim);
				continue;
			case 's':
				{
					struct tm	tm;
					char		buf[INT_STRLEN_MAXIMUM(
								time_t) + 1];
					time_t		mkt;

					tm = *t;
					mkt = mktime(&tm);
					if (TYPE_SIGNED(time_t))
						(void) sprintf(buf, "%ld",
							(long) mkt);
					else	(void) sprintf(buf, "%lu",
							(unsigned long) mkt);
					pt = _add(buf, pt, ptlim);
				}
				continue;
			case 'T':
				pt = _fmt("%H:%M:%S", t, pt, ptlim);
				continue;
			case 't':
				pt = _add("\t", pt, ptlim);
				continue;
			case 'U':
				pt = _conv((t->tm_yday + 7 - t->tm_wday) / 7,
					"%02d", pt, ptlim);
				continue;
			case 'u':
				/*
				** From Arnold Robbins' strftime version 3.0:
				** "ISO 8601: Weekday as a decimal number
				** [1 (Monday) - 7]"
				** (ado, 5/24/93)
				*/
				pt = _conv((t->tm_wday == 0) ? 7 : t->tm_wday,
					"%d", pt, ptlim);
				continue;
			case 'V':	/* ISO 8601 week number */
			case 'G':	/* ISO 8601 year (four digits) */
			case 'g':	/* ISO 8601 year (two digits) */
/*
** From Arnold Robbins' strftime version 3.0:  "the week number of the
** year (the first Monday as the first day of week 1) as a decimal number
** (01-53)."
** (ado, 1993-05-24)
**
** From "http://www.ft.uni-erlangen.de/~mskuhn/iso-time.html" by Markus Kuhn:
** "Week 01 of a year is per definition the first week which has the
** Thursday in this year, which is equivalent to the week which contains
** the fourth day of January. In other words, the first week of a new year
** is the week which has the majority of its days in the new year. Week 01
** might also contain days from the previous year and the week before week
** 01 of a year is the last week (52 or 53) of the previous year even if
** it contains days from the new year. A week starts with Monday (day 1)
** and ends with Sunday (day 7).  For example, the first week of the year
** 1997 lasts from 1996-12-30 to 1997-01-05..."
** (ado, 1996-01-02)
*/
				{
					int	year;
					int	yday;
					int	wday;
					int	w;

					year = t->tm_year + TM_YEAR_BASE;
					yday = t->tm_yday;
					wday = t->tm_wday;
					for ( ; ; ) {
						int	len;
						int	bot;
						int	top;

						len = isleap(year) ?
							DAYSPERLYEAR :
							DAYSPERNYEAR;
						/*
						** What yday (-3 ... 3) does
						** the ISO year begin on?
						*/
						bot = ((yday + 11 - wday) %
							DAYSPERWEEK) - 3;
						/*
						** What yday does the NEXT
						** ISO year begin on?
						*/
						top = bot -
							(len % DAYSPERWEEK);
						if (top < -3)
							top += DAYSPERWEEK;
						top += len;
						if (yday >= top) {
							++year;
							w = 1;
							break;
						}
						if (yday >= bot) {
							w = 1 + ((yday - bot) /
								DAYSPERWEEK);
							break;
						}
						--year;
						yday += isleap(year) ?
							DAYSPERLYEAR :
							DAYSPERNYEAR;
					}
#ifdef XPG4_1994_04_09
					if ((w == 52
					     && t->tm_mon == TM_JANUARY)
					    || (w == 1
						&& t->tm_mon == TM_DECEMBER))
						w = 53;
#endif /* defined XPG4_1994_04_09 */
					if (*format == 'V')
						pt = _conv(w, "%02d",
							pt, ptlim);
					else if (*format == 'G')
						pt = _conv(year, "%02d",
							pt, ptlim);
					else	pt = _conv(year, "%04d",
							pt, ptlim);
				}
				continue;
			case 'v':
				/*
				** From Arnold Robbins' strftime version 3.0:
				** "date as dd-bbb-YYYY"
				** (ado, 5/24/93)
				*/
				pt = _fmt("%e-%b-%Y", t, pt, ptlim);
				continue;
			case 'W':
				pt = _conv((t->tm_yday + 7 -
					(t->tm_wday ?
					(t->tm_wday - 1) : 6)) / 7,
					"%02d", pt, ptlim);
				continue;
			case 'w':
				pt = _conv(t->tm_wday, "%d", pt, ptlim);
				continue;
			case 'X':
				pt = _fmt(Locale->X_fmt, t, pt, ptlim);
				continue;
			case 'x':
				pt = _fmt(Locale->x_fmt, t, pt, ptlim);
				continue;
			case 'y':
				pt = _conv((t->tm_year + TM_YEAR_BASE) % 100,
					"%02d", pt, ptlim);
				continue;
			case 'Y':
				pt = _conv(t->tm_year + TM_YEAR_BASE, "%04d",
					pt, ptlim);
				continue;
			case 'Z':
#ifdef TM_ZONE
				if (t->TM_ZONE != NULL)
					pt = _add(t->TM_ZONE, pt, ptlim);
				else
#endif /* defined TM_ZONE */
				if (t->tm_isdst == 0 || t->tm_isdst == 1) {
					pt = _add(tzname[t->tm_isdst],
						pt, ptlim);
				} else  pt = _add("?", pt, ptlim);
				continue;
			case '+':
				pt = _fmt(Locale->date_fmt, t, pt, ptlim);
				continue;
			case '%':
			/*
			 * X311J/88-090 (4.12.3.5): if conversion char is
			 * undefined, behavior is undefined.  Print out the
			 * character itself as printf(3) also does.
			 */
			default:
				break;
			}
		}
		if (pt == ptlim)
			break;
		*pt++ = *format;
	}
	return pt;
}

static char *
_conv(n, format, pt, ptlim)
const int		n;
const char * const	format;
char * const		pt;
const char * const	ptlim;
{
	char	buf[INT_STRLEN_MAXIMUM(int) + 1];

	(void) sprintf(buf, format, n);
	return _add(buf, pt, ptlim);
}

static char *
_add(str, pt, ptlim)
const char *		str;
char *			pt;
const char * const	ptlim;
{
	while (pt < ptlim && (*pt = *str++) != '\0')
		++pt;
	return pt;
}

#ifdef LOCALE_HOME
static struct lc_time_T *
_loc P((void))
{
	static const char	locale_home[] = LOCALE_HOME;
	static const char	lc_time[] = "LC_TIME";
	static char *		locale_buf;
	static char		locale_buf_C[] = "C";

	int			fd;
	int			oldsun;	/* "...ain't got nothin' to do..." */
	char *			lbuf;
	char *			name;
	char *			p;
	const char **		ap;
	const char *		plim;
	char			filename[FILENAME_MAX];
	struct stat		st;
	size_t			namesize;
	size_t			bufsize;

	/*
	** Use localebuf.mon[0] to signal whether locale is already set up.
	*/
	if (localebuf.mon[0])
		return &localebuf;
	name = setlocale(LC_TIME, (char *) NULL);
	if (name == NULL || *name == '\0')
		goto no_locale;
	/*
	** If the locale name is the same as our cache, use the cache.
	*/
	lbuf = locale_buf;
	if (lbuf != NULL && strcmp(name, lbuf) == 0) {
		p = lbuf;
		for (ap = (const char **) &localebuf;
			ap < (const char **) (&localebuf + 1);
				++ap)
					*ap = p += strlen(p) + 1;
		return &localebuf;
	}
	/*
	** Slurp the locale file into the cache.
	*/
	namesize = strlen(name) + 1;
	if (sizeof(filename) <
		sizeof(locale_home) + namesize + sizeof(lc_time))
			goto no_locale;
	oldsun = 0;
	(void) sprintf(filename, "%s/%s/%s", locale_home, name, lc_time);
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		/*
		** Old Sun systems have a different naming and data convention.
		*/
		oldsun = 1;
		(void) sprintf(filename, "%s/%s/%s", locale_home,
			lc_time, name);
		fd = open(filename, O_RDONLY);
		if (fd < 0)
			goto no_locale;
	}
	if (fstat(fd, &st) != 0)
		goto bad_locale;
	if (st.st_size <= 0)
		goto bad_locale;
	bufsize = namesize + st.st_size;
	locale_buf = NULL;
	lbuf = (lbuf == NULL || lbuf == locale_buf_C) ?
		malloc(bufsize) : realloc(lbuf, bufsize);
	if (lbuf == NULL)
		goto bad_locale;
	(void) strcpy(lbuf, name);
	p = lbuf + namesize;
	plim = p + st.st_size;
	if (read(fd, p, (size_t) st.st_size) != st.st_size)
		goto bad_lbuf;
	if (close(fd) != 0)
		goto bad_lbuf;
	/*
	** Parse the locale file into localebuf.
	*/
	if (plim[-1] != '\n')
		goto bad_lbuf;
	for (ap = (const char **) &localebuf;
		ap < (const char **) (&localebuf + 1);
			++ap) {
				if (p == plim)
					goto bad_lbuf;
				*ap = p;
				while (*p != '\n')
					++p;
				*p++ = '\0';
	}
	if (oldsun) {
		/*
		** SunOS 4 used an obsolescent format; see localdtconv(3).
		** c_fmt had the ``short format for dates and times together''
		** (SunOS 4 date, "%a %b %e %T %Z %Y" in the C locale);
		** date_fmt had the ``long format for dates''
		** (SunOS 4 strftime %C, "%A, %B %e, %Y" in the C locale).
		** Discard the latter in favor of the former.
		*/
		localebuf.date_fmt = localebuf.c_fmt;
	}
	/*
	** Record the successful parse in the cache.
	*/
	locale_buf = lbuf;

	return &localebuf;

bad_lbuf:
	free(lbuf);
bad_locale:
	(void) close(fd);
no_locale:
	localebuf = C_time_locale;
	locale_buf = locale_buf_C;
	return &localebuf;
}
#endif /* defined LOCALE_HOME */
