#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qfloat.h"
typedef	long	 double	LDBL;
typedef	unsigned short	USHORT;

extern	LDBL	powernl(LDBL, int);
typedef	union  long_double
{
	LDBL	Number;
	USHORT	Parts[5];
}
	LDBL_UN;

LDBL_UN	NewValue;
int
ChkToExp(char *G, char *E)
{
	int	k, Match = 1;
	if (*E == ' ')
		strcpy(E, E+1);
	for (k = 0; k < strlen(E); ++k)
	{
		if (E[k] == 'E')
			break;
		Match = G[k] == E[k];
		if (Match)
			continue;
		break;
	}
	return Match;
}
int
main()
{
	int	k, m, Match, MsMtchCt = 0;
	LDBL	X;
	char	EString[128], GString[128];

	for (m = -20; m <= 20; ++m)
	{
		qfloat	QX;
		X = powernl(10.0L, m) - 1.0L;
		QX = X;
		sprintf(GString, "%.20Le", X);
		xsprintf("", QX, 20, "", EString);
		//printf("printf: %s\ne64toa: %s\n", GString, EString);
		Match = ChkToExp(GString, EString);
		if (! Match)
		{
			++MsMtchCt;
			printf("Not a printf match\n");
			printf("\t%s\n\t%s\n", GString, EString);

		}
# if 0
		NewValue.Number = X;
		printf("X = %+.20Le =", X);
		for (k = 0; k < 5; ++k)
			printf("%5.4hx", NewValue.Parts[k]);
		printf("\n");
# endif
	}
	printf("Number of printf mismatches: %d\n", MsMtchCt);
	return (0);
}
/* ============ */
/* powernl.c	*/
/* ============ */
#define	ODD(k)	((k) & 1)
/* ====================================================	*/
/* powernl - returns long double x raised to power n	*/
/* ====================================================	*/
LDBL
powernl(LDBL x, int n)
{
    int     i;
    LDBL    r;

    if (n == 0)
    {
	return(1.0);
    }

    for (i = abs(n); !ODD(i); i >>= 1)
    {
	x = x * x;
    }

    r = x;

    while (i != 1)
    {
	do
	{
	    x = x * x;
	}
	while (!(ODD(i >>= 1)));

	r = r * x;
    }

    if (n > 0)
    {
	return(r);
    }
    else
    {
	return(1 / r);
    }
}
