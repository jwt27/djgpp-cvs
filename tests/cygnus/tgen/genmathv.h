// ----------------------------------------------
// genmathv.h - Header for Test-Vector Generators
// ----------------------------------------------
# if defined _GEN_MATHV_H_
#	// Do Nothing
# else
#	define _GEN_MATHV_H_

#define	__NO_ERROR_	0

#define	EQUAL(s1, s2)	(strcmp(s1, s2) == 0)

#define	GEN_FIRST_TWO_LINES	\
    printf("#include \"test.h\"\n one_line_type %s_vec[] = {\n", Name);


typedef struct	LoopLimits
    {
	double	Start;
	double	Step;
	double	Stop;
    }
	LOOP_LIMITS;

#ifdef USE_WRITE_2_VECTOR
// --------------------------------------------------------------
// Write2Vector - Writes next two-parameter test vector to stdout
// --------------------------------------------------------------
static	void
Write2Vector(int MeasBit, double ArgY, double ArgX, double Exp,
		int ErrExp)
{
    volatile __ieee_double_shape_type  Arg1, Arg2, Expected;

    Arg1.value = ArgY, Arg2.value = ArgX, Expected.value = Exp;

    printf("{%d, %d, 0,__LINE__, 0x%08lx, 0x%08lx, "
		"0x%08lx, 0x%08lx, 0x%08lx, 0x%08lx,},"
		"\t/* %+.4E%s=F(%+8.4g,%+8.4g) */\n", MeasBit, ErrExp,
		Expected.parts.msw, Expected.parts.lsw,
		Arg1.parts.msw, Arg1.parts.lsw,
		Arg2.parts.msw, Arg2.parts.lsw,
		Expected.value,
		(isinf(Expected.value) ||
		 isnan(Expected.value)) ? "       " : "",
		Arg1.value, Arg2.value);
}
#else // USE_WRITE_2_VECTOR
// -------------------------------------------------------------
// WriteVector - Writes next one-parameter test vector to stdout
// -------------------------------------------------------------
static	void
WriteVector(int MeasBit, double Arg, double Exp, int ErrExp)
{
    volatile __ieee_double_shape_type  Argument, Expected;

    Argument.value = Arg, Expected.value = Exp;

    printf("{%d, %d, 0,__LINE__, 0x%08lx, 0x%08lx, 0x%08lx, 0x%08lx, },"
		"\t/* %+.4E%s=F(%+8.4g) */\n", MeasBit, ErrExp,
		Expected.parts.msw, Expected.parts.lsw,
		Argument.parts.msw, Argument.parts.lsw,
		Expected.value,
		(isinf(Expected.value) ||
		 isnan(Expected.value)) ? "       " : "",
		Argument.value);
}
#endif // USE_WRITE_2_VECTOR
// -------------------------------------------------------------
// AssignOutputFile - Reopens stdout as a diskfile based on Name
// -------------------------------------------------------------
void
AssignOutputFile(char *Name)
{
    FILE     *OutPtr;
    char     OutNam[128];
    strcpy(OutNam, Name);
    strcat(OutNam, "_vec.c");
    OutPtr = freopen(OutNam, "w", stdout);
    if (OutPtr == NULL)
    {
	fprintf(stderr, "In AssignOutputFile() in File %s:\n", __FILE__);
	perror(OutNam);
	exit(1);
    }
}
# endif /* ! defined _GEN_MATHV_H */
