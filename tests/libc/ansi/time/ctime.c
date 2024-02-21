/* ctime.c - basic ISO and POSIX time function calls */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int
main( void )
{
    const char *tz[] = { "AZOT-1AZOST", "WET0WEST", "CET1CEST" };
    unsigned    z;
    int         o;

    for (z = 0; z < sizeof tz / sizeof *tz; ++z)
    {
	setenv( "TZ", tz[z], 1);

	for (o = 0; o <= 4; ++o)
	{
	    time_t	t;
	    struct tm *	tmp;
	    struct tm  	tm;
	    char        buf[BUFSIZ];

	    t	= time( &t );
	    t	= 1024U*1024*1024/4*3*o;
	    printf( "time(        %10u )   = %s\n", t, getenv( "TZ" ));
	    tmp = gmtime( &t );
	    printf( "gmtime(      %10u )   = %s",   t, asctime( tmp ));
	    tmp = localtime( &t );
	    printf( "localtime(   %10u )   = %s",   t, asctime( tmp ));
	    printf( "ctime(       %10u )   = %s",   t, ctime( &t ));
	    t = mktime( tmp );
	    strftime( buf, sizeof buf, "%c%n", tmp);
	    printf( "strftime(    %10u )   = %s",   t, buf);
	    printf( "difftime(    %10u, 0) = %.0fs\n", t, difftime( t, 0));

	    t	= 1024U*1024*1024/4*3*o;
	    tmp = gmtime_r( &t, &tm);
	    printf( "gmtime_r(    %10u, t) = %s",   t, asctime_r( tmp, buf));
	    tmp = localtime_r( &t, &tm);
	    printf( "localtime_r( %10u, t) = %s",   t, asctime_r( tmp, buf));
	    printf( "ctime_r(     %10u, s) = %s\n", t, ctime_r( &t, buf));
	}
    }

    return 0;

} /* main() */
