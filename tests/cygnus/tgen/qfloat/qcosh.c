/*							qcosh.c */

#include "qhead.h"

extern QELT qone[];

int qcosh(x,y)
QELT x[], y[];
{
QELT h[NQ], w[NQ];

qexp( x, w );		/* w = exp(x) */
qdiv( w, qone, h );	/* 1/exp(x) */
qadd( w, h, y );
y[1] -= 1;
return 0;
}
