%{
/*
 * function prototype parser
 * Author: Stas Sergeev
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define YYDEBUG 1

static void yyerror(const char* s);
int yylex(void);

static int thunk_type;

static int arg_num;
static int arg_offs;
static int arg_size;
static int al_arg_size;
static int arr_sz;
static int is_ptr;
static int is_rptr;
static int is_far;
static int is_rfar;
static int is_ffar;
static int is_void;
static int is_rvoid;
static int is_const;
static int is_pas;
static int is_init;
static int is_noret;
static int rlen;
static char abuf[1024];
static char atype[256];
static char atype2[256];
static char atype3[256];
static char rtbuf[256];
/* conversion types for flat pointers */
enum { CVTYPE_OTHER, CVTYPE_VOID, CVTYPE_CHAR, CVTYPE_ARR };
static int cvtype;


static void beg_arg(void)
{
    is_far = 0;
    is_ptr = 0;
    is_void = 0;
    is_const = 0;
    cvtype = CVTYPE_OTHER;
    arr_sz = 0;
    atype[0] = 0;
    atype2[0] = 0;
    atype3[0] = 0;
}

static void init_line(void)
{
    is_init = 0;
    is_pas = 0;
    is_rvoid = 0;
    is_rptr = 0;
    is_rfar = 0;
    is_ffar = 0;
    is_noret = 0;
    rlen = 0;
    beg_arg();
}

static void do_start_arg(int anum)
{
    if (thunk_type == 1 || thunk_type == 2)
	strcat(abuf, "_");
    if (is_ptr) {
	if (is_far) {
	    switch (anum) {
	    case 0:
		strcat(abuf, "_ARG_PTR_FAR(");
		break;
	    case 1:
		strcat(abuf, "_ARG_PTR_FAR_A(");
		break;
	    case 2:
		strcat(abuf, "_CNV_PTR_FAR, _L_NONE");
		break;
	    }
	} else {
	    switch (anum) {
	    case 0:
		strcat(abuf, "_ARG_PTR(");
		break;
	    case 1:
		strcat(abuf, "_ARG_PTR_A(");
		break;
	    case 2:
		/* flat pointers need conversion to far */
		switch (cvtype) {
		case CVTYPE_VOID:
		    sprintf(abuf + strlen(abuf), "_CNV_PTR_VOID, _L_REF(%i)", arg_num + 2);
		    break;
		case CVTYPE_CHAR:
		    if (is_const)
			strcat(abuf, "_CNV_PTR_CCHAR, _L_NONE");
		    else
			strcat(abuf, "_CNV_PTR_CHAR, _L_UNIMP");
		    break;
		case CVTYPE_ARR:
		    sprintf(abuf + strlen(abuf), "_CNV_PTR_ARR, _L_IMM(%i, %i)",
			    arg_num + 1, arr_sz);
		    break;
		case CVTYPE_OTHER:
		    strcat(abuf, "_CNV_PTR, _L_NONE");
		    break;
		}
		break;
	    }
	}
    } else {
	switch (anum) {
	case 0:
	    strcat(abuf, "_ARG(");
	    break;
	case 1:
	    strcat(abuf, "_ARG_A(");
	    break;
	case 2:
	    strcat(abuf, "_CNV_SIMPLE, _L_NONE");
	    break;
	}
    }
}

static void fin_arg(int last)
{
    int real_arg_size;
    if (!atype[0])
	return;
    if (!is_ptr && is_void)
	return;
    do_start_arg(0);
    if (is_const)
	strcat(abuf, "const ");
    switch (thunk_type) {
    case 0:
	sprintf(abuf + strlen(abuf), "%i, %s, _SP)", arg_offs, atype);
	break;
    case 1:
    case 2:
	sprintf(abuf + strlen(abuf), "%s)", atype);
	strcat(abuf, ", ");
	do_start_arg(1);
	if (is_const)
	    strcat(abuf, "const ");
	sprintf(abuf + strlen(abuf), "%s)", atype2[0] ? atype2 : atype);
	strcat(abuf, ", ");
	do_start_arg(1);
	if (is_const)
	    strcat(abuf, "const ");
	if (is_ptr)
	    sprintf(abuf + strlen(abuf), "%s)", atype2[0] ? atype2 : atype);
	else
	    sprintf(abuf + strlen(abuf), "%s)", atype3[0] ?
		atype3 : (atype2[0] ? atype2 : atype));
	strcat(abuf, ", ");
	do_start_arg(2);
	break;
    }
    if (is_ptr) {
	if (is_far)
	    real_arg_size = 4;
	else
	    real_arg_size = 2;
    } else {
	if (arg_size <= 0) {
	    if (arg_size == 0 && arg_num)
		yyerror("parse error, void argument?");
	    if (arg_num == -1 && !last)
		yyerror("unknown argument size");
	    return;
	}
	real_arg_size = al_arg_size;
    }
    assert(real_arg_size > 0);
    arg_offs += real_arg_size;
    arg_num++;
}

static void add_flg(char *buf, const char *flg, int num)
{
    if (!num) {
	strcpy(buf, flg);
    } else {
	strcat(buf, " | ");
	strcat(buf, flg);
    }
}

static const char *get_flags(void)
{
    static char buf[32];
    int flg = 0;

    if (is_ffar)
	add_flg(buf, "_TFLG_FAR", flg++);
    if (is_noret)
	add_flg(buf, "_TFLG_NORET", flg++);
    if (is_init)
	add_flg(buf, "_TFLG_INIT", flg++);
    if (!flg)
	add_flg(buf, "_TFLG_NONE", flg++);
    return buf;
}

%}

%token LB RB SEMIC COMMA ASTER NEWLINE STRING NUM
%token ASMCFUNC ASMPASCAL FAR SEGM INITTEXT
%token VOID WORD UWORD CHAR BYTE UBYTE DWORD UDWORD STRUCT
%token LBR RBR
%token CONST
%token NORETURN

%define api.value.type union
%type <int> num lnum NUM
%type <char *> fname sname tname STRING

%%

input:		  input line NEWLINE
		|
;

line:		lnum rdecls fname lb args rb SEMIC
			{
			  switch (thunk_type) {
			  case 0:
			    if (!is_rvoid) {
			      if (abuf[0])
			        printf("\tcase %i:\n\t\t_DISPATCH(%i, %s, %s);\n\t\tbreak;\n",
				    $1, rlen, $3, abuf);
			      else
			        printf("\tcase %i:\n\t\t_DISPATCH(%i, %s);\n\t\tbreak;\n",
				    $1, rlen, $3);
			    } else {
			      if (abuf[0])
			        printf("\tcase %i:\n\t\t_DISPATCH_v(%s, %s);\n\t\tbreak;\n",
				    $1, $3, abuf);
			      else
			        printf("\tcase %i:\n\t\t_DISPATCH_v(%s);\n\t\tbreak;\n",
				    $1, $3);
			    }
			    break;
			  case 1:
			    /* for m4 */
			    printf("THUNK(%i, %i, %i)\n",
			        arg_num, is_rvoid && !is_rptr, is_pas);
			    break;
			  case 2:
			    /* for cpp */
			    if (!is_rvoid || is_rptr)
			      printf("_THUNK%s%i(%i, %s(%s), %s, %s",
			          is_pas ? "_P_" : "",
			          arg_num,
			          $1,
			          is_rptr ? (is_rfar ? "__ARG_PTR_FAR" :
			                  "__ARG_PTR") : "__ARG",
			          rtbuf,
			          is_rptr ? (is_rfar ? "__RET_PTR_FAR" :
			                  "__RET_PTR") : "__RET",
			          $3);
			    else
			      printf("_THUNK%s%i_v(%i, %s",
			          is_pas ? "_P_" : "",
			          arg_num,
			          $1, $3);
			    if (arg_num)
			      printf(", %s", abuf);
			    printf(", %s)\n", get_flags());
			    break;
			  }
			}
;

lb:		LB	{ arg_offs = 0; arg_num = 0; beg_arg(); }
;
rb:		RB	{ fin_arg(1); }
;

lnum:		num	{ init_line(); }
;
num:		NUM
;
fname:		STRING
;
sname:		STRING
;
tname:		STRING
;

rquals:		  FAR ASTER	{ is_rfar = 1; is_rptr = 1; }
		| ASTER		{ is_rptr = 1; }
;

quals:		  FAR quals	{ is_far = 1; }
		| ASTER quals	{ is_ptr = 1; }
		| arr
		|
;

arr:		  LBR num RBR	{ is_ptr = 1; cvtype = CVTYPE_ARR; arr_sz = $2; }
;

fatr:		  ASMCFUNC
		| ASMPASCAL	{ is_pas = 1; }
		| INITTEXT	{ is_init = 1; }
		| NORETURN	{ is_noret = 1; }
		| FAR		{ is_ffar = 1; }
		| SEGM LB STRING RB
;

fatrs:		  fatr fatrs
		| fatr
;

rq_fa:		  rquals fatrs
		| rquals
		| fatrs
		|
;

rtype:		  VOID		{ rlen = 0;
				  strcpy(rtbuf, "void");
				  is_rvoid = 1;
				}
		| WORD		{ rlen = 2;
				  strcpy(rtbuf, "WORD");
				}
		| UWORD		{ rlen = 2;
				  strcpy(rtbuf, "UWORD");
				}
		| DWORD		{ rlen = 4;
				  strcpy(rtbuf, "DWORD");
				}
		| UDWORD	{ rlen = 4;
				  strcpy(rtbuf, "UDWORD");
				}
		| BYTE		{ rlen = 1;
				  strcpy(rtbuf, "BYTE");
				}
		| CHAR		{ rlen = 1;
				  strcpy(rtbuf, "char");
				}
		| UBYTE		{ rlen = 1;
				  strcpy(rtbuf, "UBYTE");
				}
;

atype:		  VOID		{
				  arg_size = 0;
				  cvtype = CVTYPE_VOID;
				  strcat(atype, "VOID");
				  al_arg_size = 0;
				  is_void = 1;
				}
		| CHAR		{
				  arg_size = 1;
				  cvtype = CVTYPE_CHAR;
				  strcat(atype, "char");
				  strcat(atype3, "WORD");
				  al_arg_size = 2;
				}
		| WORD		{
				  arg_size = 2;
				  strcat(atype, "WORD");
				  al_arg_size = 2;
				}
		| UWORD		{
				  arg_size = 2;
				  strcat(atype, "UWORD");
				  al_arg_size = 2;
				}
		| DWORD		{
				  arg_size = 4;
				  strcat(atype, "DWORD");
				  al_arg_size = 4;
				}
		| UDWORD	{
				  arg_size = 4;
				  strcat(atype, "UDWORD");
				  al_arg_size = 4;
				}
		| BYTE		{
				  arg_size = 1;
				  strcat(atype, "BYTE");
				  strcat(atype3, "WORD");
				  al_arg_size = 2;
				}
		| UBYTE		{
				  arg_size = 1;
				  strcat(atype, "UBYTE");
				  strcat(atype3, "UWORD");
				  al_arg_size = 2;
				}
		| STRUCT sname	{
				  arg_size = -1;
				  sprintf(atype + strlen(atype), "struct %s", $2);
				}
		| tname		{
				  arg_size = -1;
				  sprintf(atype + strlen(atype), "%s", $1);
				}
;

rdecls:		rtype rq_fa	{ abuf[0] = 0; }
;

adecls:		  atype quals
		| CONST atype quals	{ is_const = 1; }
;

argsep:		COMMA		{ fin_arg(0); strcat(abuf, ", "); beg_arg(); }

args:		  args argsep arg
		| arg
;

arg:		  adecls STRING arr
		| adecls STRING
		| adecls
;

%%

int main(int argc, char *argv[])
{
    yydebug = 0;

    if (argc >= 2)
	thunk_type = atoi(argv[1]);
    if (argc >= 3 && argv[2][0] == 'd')
	yydebug = 1;
    yyparse();
    return 0;
}

static void yyerror(const char* s)
{
    fprintf(stderr, "Parse error: %s\n", s);
    exit(1);
}
