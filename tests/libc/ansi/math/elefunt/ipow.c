/* -*-C-*- ipow.c */

#include "elefunt.h"

float
ipow(x, n)
float x;
int n;
{
    float value;
    int k;

    value = 1.0;
    for (k = 1; k <= ABS(n); ++k)
	value *= x;
    if (n < 0)
	value = 1.0 / value;
    return (value);
}

