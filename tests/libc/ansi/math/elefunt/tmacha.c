/* -*-C-*- tsmach.c */

#include "elefunt.h"

static void dump(float x);

int
main()
{
    int ibeta,
        iexp,
        irnd,
        it,
        machep,
        maxexp,
        minexp,
        negep,
        ngrd;
    float
        eps,
        epsneg,
	xmax,
	xmin;
    machar(&ibeta, &it, &irnd, &ngrd, &machep, &negep, &iexp, &minexp,
	&maxexp, &eps, &epsneg, &xmin, &xmax);

    printf("ibeta............%d\n",ibeta);
    printf("it...............%d\n",it);
    printf("irnd.............%d\n",irnd);
    printf("ngrd.............%d\n",ngrd);
    printf("machep...........%d\n",machep);
    printf("negep............%d\n",negep);
    printf("iexp.............%d\n",iexp);
    printf("minexp...........%d\n",minexp);
    printf("maxexp...........%d\n",maxexp);
    printf("eps.............." F15P5E "\n",eps);
    dump(eps);
    printf("epsneg..........." F15P5E "\n",epsneg);
    dump(epsneg);
    printf("xmin............." F15P5E "\n",xmin);
    dump(xmin);
    printf("xmax............." F15P5E "\n",xmax);
    dump(xmax);
    return (EXIT_SUCCESS);
}

static void
dump(float x)
{
    unsigned char *c = (unsigned char *)&x;
    int s = sizeof(x), i;
    for (i=0; i<s; i++)
	printf("%02x,", (unsigned)c[i]);
    printf("\n");
}
