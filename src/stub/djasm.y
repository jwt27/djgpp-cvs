/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
%{

#define YYDEBUG 1
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <coff.h>
#include <fcntl.h>
#include <time.h>

#define SMALL_EXE_HEADER 0
#if SMALL_EXE_HEADER
#define EXE_HEADER_SIZE 32
#define EXE_HEADER_BLOCKS 1
#else
#define EXE_HEADER_SIZE 512
#define EXE_HEADER_BLOCKS 32
#endif

#define YYERROR_VERBOSE

void yyerror(char *s);

#define OUT_exe 0
#define OUT_com 1
#define OUT_bin 2
#define OUT_h   3
#define OUT_inc 4
#define OUT_s   5

char *ext_types[] = {
  "exe",
  "com",
  "bin",
  "h",
  "inc",
  "ah",
  0
};

char *INC_LEADER = "\t.db\t";
char *S_LEADER = "\t.byte\t";

int out_type = OUT_exe;
int image_type = OUT_exe;
char *outname = 0;

int i;
int lineno = 1;
char *inname;
int total_errors = 0;
char last_token[100];
int last_tret;
char *copyright = 0;

int last_align_begin=-1, last_align_end=-1;
int generated_bytes = -1;

char strbuf[200];
int strbuflen;

typedef struct {
  short line;
  short addr;
} lineaddr_s;

lineaddr_s *lineaddr=0;
int num_lineaddr=0;
int max_lineaddr=0;

unsigned char *outbin = 0;
int outsize = 0;
int pc = 0;
int bsspc = -1;
int stack_ptr = 0;
int start_ptr = 0;

typedef struct Symbol {
  struct Symbol *next;
  char *name;
  int value;
  int defined;
  struct Patch *patches;
  int first_used;
  int type;
} Symbol;

#define SYM_unknown	0
#define SYM_abs		1
#define SYM_data	2
#define SYM_code	4
#define SYMTYPES "?ADDTTTT"

#define REL_abs		0
#define REL_abs32	1
#define REL_16		2
#define REL_8		3

typedef struct Patch {
  struct Patch *next;
  int location;
  int lineno;
  int rel;
} Patch;

Symbol *symtab = 0;
Symbol *get_symbol(char *name, int create);
Symbol *set_symbol(Symbol *sym, int value);
Symbol *zerosym;

struct {
  int regs;
  int offset;
  int nsyms;
  Symbol *syms[10];
} _modrm = { 0, 0, 0 };

unsigned char sreg_overrides[] = {
  0x26, 0x2e, 0x36, 0x3e, 0x64, 0x65
};

void emit(void *v, int len);
void emitb(int b);
void emitw(int w);
void emitd(long d);
void emits(Symbol *s, int offset, int rel);
void modrm(int mod, int reg, int rm);
void reg(int reg);
void sortsyms();

int istemp(char *symname, char which);
void do_sreg_pop(int sreg);
void do_sreg_push(int sreg);
void do_align(int p2);
void set_lineaddr();
void add_copyright(char *buf);
void add_rcs_ident(char *buf);

void set_out_type(char *type);
void set_image_type(char *type);
void do_include(char *fname);
void do_linkcoff(char *fname);

%}

%expect 2 /* see regmem */

%union {
  Symbol *sym;
  int i;
  struct {
    Symbol *sym;
    int ofs;
  } relsym;
}

%type <sym> ID
%type <relsym> constID
%type <i> const offset

%token <sym> KID UID

%left OP_OR '|' '^'
%left OP_AND '&'
%nonassoc '=' '<' '>' OP_NE OP_LE OP_GE
%left OP_SHL OP_SHR
%left '+' '-'
%left '*' '/' '%'
%left OP_NEG OP_LNOT OP_NOT

%token <i> NUMBER REG8 REG16 REG32 SREG STRING PC CRREG DRREG TRREG
%token <i> ARITH2 ARITH2B ARITH2D ARITH2W
%token <i> LXS MOVSZX
%token <i> JCC JCCL LOOP
%token <i> SHIFT SHLRD
%token <i> ONEBYTE TWOBYTE
%token <i> BITTEST GROUP3 GROUP6 GROUP7
%token ALIGN
%token BSS
%token CALL CALLF COPYRIGHT
%token DB DD DEC DECB DECD DECW DUP DW
%token IN INC INCB INCD INCW INT INCLUDE
%token JMPW JMPB JMPF
%token LINKCOFF
%token MOV MOVB MOVD MOVW
%token ORG OUT
%token POP POPW POPD PUSH PUSHW PUSHD
%token RCS_ID RET RETF
%token STACK START
%token TEST TESTB TESTD TESTW TYPE
%token XCHG

%{
#define NO_ATTR -1

struct opcode {
  char *name;
  int token;
  int attr;
};

struct opcode opcodes[] = {
  "aaa", ONEBYTE, 0x37,
  "aad", TWOBYTE, 0xd50a,
  "aam", TWOBYTE, 0xd40a,
  "aas", ONEBYTE, 0x3f,
  "cbw", ONEBYTE, 0x98,
  "cwde", TWOBYTE, 0x6698,
  "clc", ONEBYTE, 0xf8,
  "cld", ONEBYTE, 0xfc,
  "cli", ONEBYTE, 0xfa,
  "clts", TWOBYTE, 0x0f06,
  "cmc", ONEBYTE, 0xf5,
  "cmpsb", ONEBYTE, 0xa6,
  "cmpsw", ONEBYTE, 0xa7,
  "cmpsd", TWOBYTE, 0x66a7,
  "cpuid", TWOBYTE, 0x0fa2,
  "cwd", ONEBYTE, 0x99,
  "cdq", TWOBYTE, 0x6699,
  "daa", ONEBYTE, 0x27,
  "das", ONEBYTE, 0x2f,
  "hlt", ONEBYTE, 0xf4,
  "insb", ONEBYTE, 0x6c,
  "insw", ONEBYTE, 0x6d,
  "insd", TWOBYTE, 0x666d,
  "into", ONEBYTE, 0xce,
  "iret", ONEBYTE, 0xcf,
  "iretd", TWOBYTE, 0x66cf,
  "lahf", ONEBYTE, 0x9f,
  "leave", ONEBYTE, 0xc9,
  "lock", ONEBYTE, 0xf0,
  "lodsb", ONEBYTE, 0xac,
  "lodsw", ONEBYTE, 0xad,
  "lodsd", TWOBYTE, 0x66ad,
  "movsb", ONEBYTE, 0xa4,
  "movsw", ONEBYTE, 0xa5,
  "movsd", TWOBYTE, 0x66a5,
  "nop", ONEBYTE, 0x90,
  "outsb", ONEBYTE, 0x6e,
  "outsw", ONEBYTE, 0x6f,
  "outsd", TWOBYTE, 0x666f,
  "popa", ONEBYTE, 0x61,
  "popad", TWOBYTE, 0x6661,
  "popf", ONEBYTE, 0x9d,
  "popfd", TWOBYTE, 0x669d,
  "pusha", ONEBYTE, 0x60,
  "pushad", TWOBYTE, 0x6660,
  "pushf", ONEBYTE, 0x9c,
  "pushfd", TWOBYTE, 0x669c,
  "rep", ONEBYTE, 0xf3,
  "repe", ONEBYTE, 0xf3,
  "repz", ONEBYTE, 0xf3,
  "repne", ONEBYTE, 0xf2,
  "repnz", ONEBYTE, 0xf2,
  "sahf", ONEBYTE, 0x9e,
  "scasb", ONEBYTE, 0xae,
  "scasw", ONEBYTE, 0xaf,
  "scasd", TWOBYTE, 0x66af,
  "stc", ONEBYTE, 0xf9,
  "std", ONEBYTE, 0xfd,
  "sti", ONEBYTE, 0xfb,
  "stosb", ONEBYTE, 0xaa,
  "stosw", ONEBYTE, 0xab,
  "stosd", TWOBYTE, 0x66ab,
  "wait", ONEBYTE, 0x9b,
  "fwait", ONEBYTE, 0x9b,
  "xlat", ONEBYTE, 0xd7,
  "xlatb", ONEBYTE, 0xd7,

  ".addrsize", ONEBYTE, 0x67,
  ".opsize", ONEBYTE, 0x66,
  ".segcs", ONEBYTE, 0x2e,
  ".segds", ONEBYTE, 0x3e,
  ".seges", ONEBYTE, 0x26,
  ".segss", ONEBYTE, 0x36,
  ".segfs", ONEBYTE, 0x64,
  ".seggs", ONEBYTE, 0x65,

  ".align", ALIGN, NO_ATTR,
  ".bss", BSS, NO_ATTR,
  ".copyright", COPYRIGHT, NO_ATTR,
  ".db", DB, NO_ATTR,
  ".dd", DD, NO_ATTR,
  ".dup", DUP, NO_ATTR,
  ".dw", DW, NO_ATTR,
  ".id", RCS_ID, NO_ATTR,
  ".include", INCLUDE, NO_ATTR,
  ".linkcoff", LINKCOFF, NO_ATTR,
  ".org", ORG, NO_ATTR,
  ".stack", STACK, NO_ATTR,
  ".start", START, NO_ATTR,
  ".type", TYPE, NO_ATTR,

  "adc", ARITH2, 2,
  "adcb", ARITH2B, 2,
  "adcd", ARITH2D, 2,
  "adcw", ARITH2W, 2,
  "add", ARITH2, 0,
  "addb", ARITH2B, 0,
  "addd", ARITH2D, 0,
  "addw", ARITH2W, 0,
  "and", ARITH2, 4,
  "andb", ARITH2B, 4,
  "andd", ARITH2D, 4,
  "andw", ARITH2W, 4,
  "bt", BITTEST, 4,
  "btc", BITTEST, 7,
  "btr", BITTEST, 6,
  "bts", BITTEST, 5,
  "call", CALL, NO_ATTR,
  "callf", CALLF, NO_ATTR,
  "cmp", ARITH2, 7,
  "cmpb", ARITH2B, 7,
  "cmpd", ARITH2D, 7,
  "cmpw", ARITH2W, 7,
  "dec", DEC, NO_ATTR,
  "decb", DECB, NO_ATTR,
  "decd", DECD, NO_ATTR,
  "decw", DECW, NO_ATTR,
  "in", IN, NO_ATTR,
  "inc", INC, NO_ATTR,
  "incb", INCB, NO_ATTR,
  "incd", INCD, NO_ATTR,
  "incw", INCW, NO_ATTR,
  "int", INT, NO_ATTR,

  "jo", JCC, 0,
  "jno", JCC, 1,
  "jb", JCC, 2,
  "jc", JCC, 2,
  "jnae", JCC, 2,
  "jnb", JCC, 3,
  "jnc", JCC, 3,
  "jae", JCC, 3,
  "jz", JCC, 4,
  "je", JCC, 4,
  "jnz", JCC, 5,
  "jne", JCC, 5,
  "jbe", JCC, 6,
  "jna", JCC, 6,
  "jnbe", JCC, 7,
  "ja", JCC, 7,
  "js", JCC, 8,
  "jns", JCC, 9,
  "jp", JCC, 10,
  "jpe", JCC, 10,
  "jnp", JCC, 11,
  "jpo", JCC, 11,
  "jl", JCC, 12,
  "jnge", JCC, 12,
  "jnl", JCC, 13,
  "jge", JCC, 13,
  "jle", JCC, 14,
  "jng", JCC, 14,
  "jnle", JCC, 15,
  "jg", JCC, 15,

  "jol", JCCL, 0,
  "jnol", JCCL, 1,
  "jbl", JCCL, 2,
  "jcl", JCCL, 2,
  "jnael", JCCL, 2,
  "jnbl", JCCL, 3,
  "jncl", JCCL, 3,
  "jael", JCCL, 3,
  "jzl", JCCL, 4,
  "jel", JCCL, 4,
  "jnzl", JCCL, 5,
  "jnel", JCCL, 5,
  "jbel", JCCL, 6,
  "jnal", JCCL, 6,
  "jnbel", JCCL, 7,
  "jal", JCCL, 7,
  "jsl", JCCL, 8,
  "jnsl", JCCL, 9,
  "jpl", JCCL, 10,
  "jpel", JCCL, 10,
  "jnpl", JCCL, 11,
  "jpol", JCCL, 11,
  "jll", JCCL, 12,
  "jngel", JCCL, 12,
  "jnll", JCCL, 13,
  "jgel", JCCL, 13,
  "jlel", JCCL, 14,
  "jngl", JCCL, 14,
  "jnlel", JCCL, 15,
  "jgl", JCCL, 15,

  "jmp", JMPB, NO_ATTR,
  "jmpf", JMPF, NO_ATTR,
  "jmpl", JMPW, NO_ATTR,
  "lds", LXS, 0xc5,
  "les", LXS, 0xc4,
  "lfs", LXS, 0x0fb4,
  "lgs", LXS, 0x0fb5,
  "lss", LXS, 0x0fb2,
  "lgdt", GROUP7, 2,
  "lidt", GROUP7, 3,
  "lldt", GROUP6, 2,
  "lmsw", GROUP7, 6,
  "loop", LOOP, 0xe2,
  "loope", LOOP, 0xe1,
  "loopne", LOOP, 0xe0,
  "loopnz", LOOP, 0xe0,
  "loopz", LOOP, 0xe1,
  "ltr", GROUP6, 3,
  "mov", MOV, NO_ATTR,
  "movb", MOVB, NO_ATTR,
  "movd", MOVD, NO_ATTR,
  "movw", MOVW, NO_ATTR,
  "movsx", MOVSZX, 0xbe,
  "movzx", MOVSZX, 0xb6,
  "not", GROUP3, 2,
  "neg", GROUP3, 3,
  "or", ARITH2, 1,
  "orb", ARITH2B, 1,
  "ord", ARITH2D, 1,
  "orw", ARITH2W, 1,
  "out", OUT, NO_ATTR,
  "pop", POP, NO_ATTR,
  "popw", POPW, NO_ATTR,
  "popd", POPD, NO_ATTR,
  "push", PUSH, NO_ATTR,
  "pushw", PUSHW, NO_ATTR,
  "pushd", PUSHD, NO_ATTR,
  "rcl", SHIFT, 2,
  "rcr", SHIFT, 3,
  "ret", RET, NO_ATTR,
  "retf", RETF, NO_ATTR,
  "rol", SHIFT, 0,
  "ror", SHIFT, 1,
  "sar", SHIFT, 7,
  "sbb", ARITH2, 3,
  "sbbb", ARITH2B, 3,
  "sbbd", ARITH2D, 3,
  "sbbw", ARITH2W, 3,
  "sgdt", GROUP7, 0,
  "sidt", GROUP7, 1,
  "sldt", GROUP6, 0,
  "shl", SHIFT, 4,
  "shld", SHLRD, 0xa4,
  "shr", SHIFT, 5,
  "shrd", SHLRD, 0xac,
  "smsw", GROUP7, 4,
  "str", GROUP6, 1,
  "sub", ARITH2, 5,
  "subb", ARITH2B, 5,
  "subd", ARITH2D, 5,
  "subw", ARITH2W, 5,
  "test", TEST, NO_ATTR,
  "testb", TESTB, NO_ATTR,
  "testw", TESTW, NO_ATTR,
  "testd", TESTD, NO_ATTR,
  "verr", GROUP6, 4,
  "verw", GROUP6, 5,
  "xchg", XCHG, NO_ATTR,
  "xor", ARITH2, 6,
  "xorb", ARITH2B, 6,
  "xord", ARITH2D, 6,
  "xorw", ARITH2W, 6,

  "al", REG8, 0,
  "cl", REG8, 1,
  "dl", REG8, 2,
  "bl", REG8, 3,
  "ah", REG8, 4,
  "ch", REG8, 5,
  "dh", REG8, 6,
  "bh", REG8, 7,

  "es", SREG, 0,
  "cs", SREG, 1,
  "ss", SREG, 2,
  "ds", SREG, 3,
  "fs", SREG, 4,
  "gs", SREG, 5,

  "ax", REG16, 0,
  "cx", REG16, 1,
  "dx", REG16, 2,
  "bx", REG16, 3,
  "sp", REG16, 4,
  "bp", REG16, 5,
  "si", REG16, 6,
  "di", REG16, 7,

  "eax", REG32, 0,
  "ecx", REG32, 1,
  "edx", REG32, 2,
  "ebx", REG32, 3,
  "esp", REG32, 4,
  "ebp", REG32, 5,
  "esi", REG32, 6,
  "edi", REG32, 7,

  "cr0", CRREG, 0,
  "cr2", CRREG, 2,
  "cr3", CRREG, 3,

  "dr0", DRREG, 0,
  "dr1", DRREG, 1,
  "dr2", DRREG, 2,
  "dr3", DRREG, 3,
  "dr6", DRREG, 6,
  "dr7", DRREG, 7,

  "tr3", TRREG, 3,
  "tr4", TRREG, 4,
  "tr5", TRREG, 5,
  "tr6", TRREG, 6,
  "tr7", TRREG, 7};
%}

%start lines

%% /***********************************************************************/

lines
	:
	| lines line '\n'		{ lineno++; }
	| lines INCLUDE STRING '\n'	{ strbuf[strbuflen]=0; do_include(strbuf); }
	| lines INCLUDE NUMBER '\n'	{ strbuf[strbuflen]=0; do_include(strbuf); }
	;

line
	:

	| ID ':'			{ set_symbol($1, pc)->type |= (pc?SYM_data:SYM_code); }
	| ID '=' const			{ set_symbol($1, $3)->type = SYM_abs; }
	| ID '=' PC offset		{ set_symbol($1, pc + $4)->type = SYM_abs; }
	| error

	| ONEBYTE			{ emitb($1); }
	| TWOBYTE			{ emitb($1>>8); emitb($1 & 0xff); }

	| BSS				{ bsspc = pc; generated_bytes = last_align_end == pc ? last_align_begin : pc; }

	| SREG ':'			{ emitb(sreg_overrides[$1]); }

	| ARITH2B regmem ',' const	{ emitb(0x80), reg($1); emitb($4); }
	| ARITH2 REG8 ',' const		{ emitb(0x80), modrm(3, $1, $2); emitb($4); }
	| ARITH2 REG8 ',' REG8		{ emitb($1*8); modrm(3, $4, $2); }
	| ARITH2 regmem ',' REG8	{ emitb($1*8); reg($4); }
	| ARITH2 REG8 ',' regmem	{ emitb($1*8+2); reg($2); }

	| ARITH2W regmem ',' constID	{ emitb(0x81); reg($1); emits($4.sym,$4.ofs,REL_abs); }
	| ARITH2 REG16 ',' constID	{ emitb(0x81); modrm(3, $1, $2); emits($4.sym,$4.ofs,REL_abs); }
	| ARITH2 REG16 ',' REG16	{ emitb($1*8+1); modrm(3, $4, $2); }
	| ARITH2 regmem ',' REG16	{ emitb($1*8+1); reg($4); }
	| ARITH2 REG16 ',' regmem	{ emitb($1*8+3); reg($2); }

	| ARITH2D regmem ',' constID	{ emitb(0x66); emitb(0x81); reg($1); emits($4.sym,$4.ofs,REL_abs); emitw($4.ofs >> 16); }
	| ARITH2 REG32 ',' constID	{ emitb(0x66); emitb(0x81); modrm(3, $1, $2); emits($4.sym,$4.ofs,REL_abs); emitw($4.ofs >> 16); }
	| ARITH2 REG32 ',' REG32	{ emitb(0x66); emitb($1*8+1); modrm(3, $4, $2); }
	| ARITH2 regmem ',' REG32	{ emitb(0x66); emitb($1*8+1); reg($4); }
	| ARITH2 REG32 ',' regmem	{ emitb(0x66); emitb($1*8+3); reg($2); }

	| ALIGN const			{ do_align($2); }

	| BITTEST REG16 ',' REG16	{ emitb(0x0f); emitb($1*8+0x83); modrm(3, $4, $2); }
	| BITTEST regmem ',' REG16	{ emitb(0x0f); emitb($1*8+0x83); reg($4); }
	| BITTEST REG32 ',' REG32	{ emitb(0x66); emitb(0x0f); emitb($1*8+0x83); modrm(3, $4, $2); }
	| BITTEST regmem ',' REG32	{ emitb(0x66); emitb(0x0f); emitb($1*8+0x83); reg($4); }
	| BITTEST REG16 ',' const	{ emitb(0x0f); emitb(0xba); modrm(3, $1, $2); emitb($4); }
	| BITTEST regmem ',' const	{ emitb(0x0f); emitb(0xba); reg($1); emitb($4); }
	| BITTEST REG32 ',' const	{ emitb(0x66); emitb(0x0f); emitb(0xba); modrm(3, $1, $2); emitb($4); }

	| CALL ID			{ emitb(0xe8); emits($2,0,REL_16); $2->type |= SYM_code; }
	| CALLF regmem			{ emitb(0xff); reg(3); }

	| COPYRIGHT STRING		{ strbuf[strbuflen] = 0; add_copyright(strbuf); }
	| RCS_ID			{ strbuf[strbuflen] = 0; add_rcs_ident(strbuf); }

	| DB dblist
	| DW dwlist
	| DD ddlist

	| DEC REG8			{ emitb(0xfe); modrm(3, 1, $2); }
	| DECB regmem			{ emitb(0xfe); reg(1); }
	| DEC REG16			{ emitb(0x48 + $2); }
	| DEC REG32			{ emitb(0x66); emitb(0x48 + $2); }
	| DECW regmem			{ emitb(0xff); reg(1); }
	| DECD regmem			{ emitb(0x66); emitb(0xff); reg(1); }

	| IN REG8 ',' const		{ emitb(0xe4); emitb($4); }
	| IN REG16 ',' const		{ emitb(0xe5); emitb($4); }
	| IN REG32 ',' const		{ emitb(0x66); emitb(0xe5); emitb($4);}
	| IN REG8 ',' REG16		{ emitb(0xec); }
	| IN REG16 ',' REG16		{ emitb(0xed); }
	| IN REG32 ',' REG16		{ emitb(0x66); emitb(0xed); }

	| INC REG8			{ emitb(0xfe); modrm(3, 0, $2); }
	| INCB regmem			{ emitb(0xfe); reg(0); }
	| INC REG16			{ emitb(0x40 + $2); }
	| INC REG32			{ emitb(0x66); emitb(0x40 + $2); }
	| INCW regmem			{ emitb(0xff); reg(0); }
	| INCD regmem			{ emitb(0x66); emitb(0xff); reg(0); }

	| GROUP3 REG8			{ emitb(0xf6); modrm(3, $1, $2); }
	| GROUP3 REG16			{ emitb(0xf7); modrm(3, $1, $2); }
	| GROUP3 REG32			{ emitb(0x66); emitb(0xf7); modrm(3, $1, $2); }

	| GROUP6 regmem			{ emitb(0x0f); emitb(0x00); reg($1); }
	| GROUP6 REG16			{ emitb(0x0f); emitb(0x00); modrm(3, $1, $2); }
	| GROUP7 regmem			{ emitb(0x0f); emitb(0x01); reg($1); }
	| GROUP7 REG16			{ emitb(0x0f); emitb(0x01); modrm(3, $1, $2); }

	| INT const			{ if ($2 == 3) emitb(0xcc); else emitb(0xcd), emitb($2); }

	| JCC ID       			{ emitb(0x70+$1); emits($2,0,REL_8); $2->type |= SYM_code; }
	| JCCL ID			{ emitb(0x0f); emitb(0x80+$1); emits($2,0,REL_16); $2->type |= SYM_code; }

	| JMPW ID			{ emitb(0xe9); emits($2,0,REL_16); $2->type |= SYM_code; }
	| JMPB ID			{ emitb(0xeb); emits($2,0,REL_8); $2->type |= SYM_code; }
	| JMPF regmem			{ emitb(0xff); reg(5); }

	| LINKCOFF STRING		{ strbuf[strbuflen]=0; do_linkcoff(strbuf); }
	| LOOP ID			{ emitb($1); emits($2,0,REL_8); }

	| LXS REG16 ',' regmem		{ if ($1>>8) emitb($1>>8); emitb($1 & 0xff); reg($2); }
	| LXS REG32 ',' regmem		{ emitb(0x66); if ($1>>8) emitb($1>>8); emitb($1 & 0xff); reg($2); }

	| MOVB regmem ',' const		{ emitb(0xc6), reg(0); emitb($4); }
	| MOV REG8 ',' const		{ emitb(0xb0+$2); emitb($4); }
	| MOV REG8 ',' REG8		{ emitb(0x88), modrm(3, $4, $2); }
	| MOV regmem ',' REG8		{ emitb(0x88); reg($4); }
	| MOV REG8 ',' regmem		{ emitb(0x8a); reg($2); }

	| MOVW regmem ',' constID	{ emitb(0xc7); reg(0); emits($4.sym,$4.ofs,REL_abs); }
	| MOV REG16 ',' constID		{ emitb(0xb8+$2); emits($4.sym,$4.ofs,REL_abs); }
	| MOV REG16 ',' REG16		{ emitb(0x89); modrm(3, $4, $2); }
	| MOV regmem ',' REG16		{ emitb(0x89); reg($4); }
	| MOV REG16 ',' regmem		{ emitb(0x8b); reg($2); }

	| MOVD regmem ',' constID	{ emitb(0x66); emitb(0xc7); reg(0); emits($4.sym,$4.ofs,REL_abs); emitw($4.ofs >> 16); }
	| MOV REG32 ',' constID		{ emitb(0x66); emitb(0xb8+$2); emits($4.sym,$4.ofs,REL_abs); emitw($4.ofs >> 16); }
	| MOV REG32 ',' REG32		{ emitb(0x66); emitb(0x89); modrm(3, $4, $2); }
	| MOV regmem ',' REG32		{ emitb(0x66); emitb(0x89); reg($4); }
	| MOV REG32 ',' regmem		{ emitb(0x66); emitb(0x8b); reg($2); }

	| MOV regmem ',' SREG		{ emitb(0x8c); reg($4); }
	| MOV REG16 ',' SREG		{ emitb(0x8c); modrm(3, $4, $2); }
	| MOV SREG ',' regmem		{ emitb(0x8e); reg($2); }
	| MOV SREG ',' REG16		{ emitb(0x8e); modrm(3, $2, $4); }

	| MOV CRREG ',' REG32		{ emitb(0x0f); emitb(0x22); modrm(3, $2, $4); }
	| MOV DRREG ',' REG32		{ emitb(0x0f); emitb(0x23); modrm(3, $2, $4); }
	| MOV TRREG ',' REG32		{ emitb(0x0f); emitb(0x26); modrm(3, $2, $4); }
	| MOV REG32 ',' CRREG		{ emitb(0x0f); emitb(0x20); modrm(3, $4, $2); }
	| MOV REG32 ',' DRREG		{ emitb(0x0f); emitb(0x21); modrm(3, $4, $2); }
	| MOV REG32 ',' TRREG		{ emitb(0x0f); emitb(0x24); modrm(3, $4, $2); }

	| MOVSZX REG16 ',' REG8		{ emitb(0x0f); emitb($1); modrm(3, $2, $4); }
	| MOVSZX REG32 ',' REG8		{ emitb(0x66); emitb(0x0f); emitb($1); modrm(3, $2, $4); }
	| MOVSZX REG32 ',' REG16	{ emitb(0x66); emitb(0x0f); emitb($1+1); modrm(3, $2, $4); }

	| ORG const			{ if (pc > $2) yyerror ("Backwards org directive"); else while (pc < $2) emitb(0x90); }

	| OUT const ',' REG8		{ emitb(0xe6); emitb($2); }
	| OUT const ',' REG16		{ emitb(0xe7); emitb($2); }
	| OUT const ',' REG32		{ emitb(0x66); emitb(0xe7); emitb($2);}
	| OUT REG16 ',' REG8		{ emitb(0xee); }
	| OUT REG16 ',' REG16		{ emitb(0xef); }
	| OUT REG16 ',' REG32		{ emitb(0x66); emitb(0xef); }

	| POP REG16			{ emitb(0x58 + $2); }
	| POP REG32			{ emitb(0x66); emitb(0x58 + $2); }
	| POP SREG			{ do_sreg_pop($2); }
	| POPW regmem			{ emitb(0x8f); reg(0); }
	| POPD regmem			{ emitb(0x66); emitb(0x8f); reg(0); }
	| PUSH REG16			{ emitb(0x50 + $2); }
	| PUSH REG32			{ emitb(0x66); emitb(0x50 + $2); }
	| PUSH SREG			{ do_sreg_push($2); }
	| PUSHW regmem			{ emitb(0xff); reg(6); }
	| PUSHD regmem			{ emitb(0x66); emitb(0xff); reg(6); }
	| PUSHW constID			{ emitb(0x68); emits($2.sym,$2.ofs,REL_abs); }
	| PUSHD constID			{ emitb(0x66); emitb(0x68); emits($2.sym,$2.ofs,REL_abs); emitw($2.ofs >> 16); }

	| RET				{ emitb(0xc3); }
	| RET const			{ emitb(0xc2); emitw($2); }
	| RETF				{ emitb(0xcb); }
	| RETF const			{ emitb(0xca); emitw($2); }

	| SHIFT REG8 ',' const		{ emitb($4 == 1 ? 0xd0 : 0xc0); modrm(3, $1, $2); if ($4 != 1) emitb($4); }
	| SHIFT REG8 ',' REG8		{ if ($4 != 1) yyerror ("Non-constant shift count must be `cl'"); emitb(0xd2); modrm(3, $1, $2); }
	| SHIFT REG16 ',' const       	{ emitb($4 == 1 ? 0xd1 : 0xc1); modrm(3, $1, $2); if ($4 != 1) emitb($4); }
	| SHIFT REG16 ',' REG8		{ if ($4 != 1) yyerror ("Non-constant shift count must be `cl'"); emitb(0xd3); modrm(3, $1, $2); }
	| SHIFT REG32 ',' const       	{ emitb(0x66); emitb($4 == 1 ? 0xd1 : 0xc1); modrm(3, $1, $2); if ($4 != 1) emitb($4); }
	| SHIFT REG32 ',' REG8		{ if ($4 != 1) yyerror ("Non-constant shift count must be `cl'"); emitb(0x66); emitb(0xd3); modrm(3, $1, $2); }

	| SHLRD REG16 ',' REG16 ',' const
	  { emitb(0x0f); emitb($1); modrm(3, $4, $2); emitb($6); }
	| SHLRD REG16 ',' REG16 ',' REG8
	  { if ($6 != 1) yyerror ("Non-constant shift count must be `cl'");
	    emitb(0x0f); emitb($1+1); modrm(3, $4, $2); }

	| STACK				{ stack_ptr = pc; }
	| START				{ start_ptr = pc; }

	| TESTB regmem ',' const	{ emitb(0xf6), reg(0); emitb($4); }
	| TEST REG8 ',' const		{ emitb(0xf6), modrm(3, 0, $2); emitb($4); }
	| TEST REG8 ',' REG8		{ emitb(0x84), modrm(3, $4, $2); }
	| TEST regmem ',' REG8		{ emitb(0x84), reg($4); }
	| TEST REG8 ',' regmem		{ emitb(0x84), reg($2); }

	| TESTW regmem ',' constID	{ emitb(0xf7); reg(0); emits($4.sym,$4.ofs,REL_abs); }
	| TEST REG16 ',' constID	{ emitb(0xf7); modrm(3, 0, $2); emits($4.sym,$4.ofs,REL_abs); }
	| TEST REG16 ',' REG16		{ emitb(0x85); modrm(3, $4, $2); }
	| TEST regmem ',' REG16		{ emitb(0x85); reg($4); }
	| TEST REG16 ',' regmem		{ emitb(0x85); reg($2); }

	| TESTD regmem ',' constID	{ emitb(0x66); emitb(0xf7); reg(0); emits($4.sym,$4.ofs,REL_abs); emitw($4.ofs >> 16); }
	| TEST REG32 ',' constID	{ emitb(0x66); emitb(0xf7); modrm(3, 0, $2); emits($4.sym,$4.ofs,REL_abs); emitw($4.ofs >> 16); }
	| TEST REG32 ',' REG32		{ emitb(0x66); emitb(0x85); modrm(3, $4, $2); }
	| TEST regmem ',' REG32		{ emitb(0x66); emitb(0x85); reg($4); }
	| TEST REG32 ',' regmem		{ emitb(0x66); emitb(0x85); reg($2); }

	| TYPE STRING			{ strbuf[strbuflen] = 0; set_image_type(strbuf); }
	| TYPE NUMBER			{ if ($2 == 'h') set_out_type("h"); }

	| XCHG REG8 ',' REG8		{ emitb(0x86); modrm(3, $2, $4); }
	| XCHG REG8 ',' regmem		{ emitb(0x86); reg($2); }
	| XCHG regmem ',' REG8		{ emitb(0x86); reg($4); }
	| XCHG REG16 ',' REG16		{ emitb(0x87); modrm(3, $2, $4); }
	| XCHG REG16 ',' regmem		{ emitb(0x87); reg($2); }
	| XCHG regmem ',' REG16		{ emitb(0x87); reg($4); }
	| XCHG REG32 ',' REG32		{ emitb(0x66); emitb(0x87); modrm(3, $2, $4); }
	| XCHG REG32 ',' regmem		{ emitb(0x66); emitb(0x87); reg($2); }
	| XCHG regmem ',' REG32		{ emitb(0x66); emitb(0x87); reg($4); }
	;

dbitem
	: const				{ emitb($1); }
	| STRING			{ emit(strbuf, strbuflen); }
	| const DUP const		{ for (i=0; i<$1; i++) emitb($3); }
	;

dblist
	: dbitem
	| dblist ',' dbitem
	;

dwitem
	: const				{ emitw($1); }
	| UID offset			{ emits($1,$2,REL_abs); }
	| const DUP const		{ for (i=0; i<$1; i++) emitw($3); }
	;

dwlist
	: dwitem
	| dwlist ',' dwitem
	;

dditem
	: const				{ emitd($1); }
	| UID offset			{ emits($1,$2,REL_abs); emitw(0); }
	| const DUP const		{ for (i=0; i<$1; i++) emitd($3); }
	;

ddlist
	: dditem
	| ddlist ',' dditem
	;

regmem
	: regmemitem '[' regmemexpr ']'
	| '[' regmemexpr ']'
	| SREG ':' '[' regmemexpr ']'	{ emitb(sreg_overrides[$1]); }
	;

regmemexpr
	: regmemitem
	| regmemexpr '+' regmemexpr
 	| regmemexpr '-' const		{ _modrm.offset -= $3; }
	;

regmemitem
	: SREG ':' regmemitem		{ emitb(sreg_overrides[$1]); }
	| REG16				{ _modrm.regs |= (1<<$1); }
	| UID				{ _modrm.syms[_modrm.nsyms++] = $1; }
	| const				{ _modrm.offset += $1; }
	;

const
	: NUMBER			{ $$ = $1; }
	| KID				{ $$ = $1->value; }
	| const OP_OR const		{ $$ = $1 || $3; }
	| const '|' const		{ $$ = $1 | $3; }
	| const '^' const		{ $$ = $1 ^ $3; }
	| const OP_AND const		{ $$ = $1 && $3; }
	| const '&' const		{ $$ = $1 & $3; }
	| const '=' const		{ $$ = $1 == $3; }
	| const '>' const		{ $$ = $1 > $3; }
	| const '<' const		{ $$ = $1 < $3; }
	| const OP_GE const		{ $$ = $1 >= $3; }
	| const OP_LE const		{ $$ = $1 <= $3; }
	| const OP_NE const		{ $$ = $1 != $3; }
	| const OP_SHL const		{ $$ = $1 << $3; }
	| const OP_SHR const		{ $$ = $1 >> $3; }
	| const '+' const		{ $$ = $1 + $3; }
	| const '-' const		{ $$ = $1 - $3; }
	| const '*' const		{ $$ = $1 * $3; }
	| const '/' const		{ $$ = $1 / $3; }
	| const '%' const		{ $$ = $1 % $3; }
	| '-' const %prec OP_NEG	{ $$ = -$2; }
	| '!' const %prec OP_NOT	{ $$ = !$2; }
	| '~' const %prec OP_LNOT	{ $$ = ~$2; }
	| '(' const ')'			{ $$ = $2; }
	;

ID
	: KID				{ $$ = $1; }
	| UID				{ $$ = $1; }
	;

constID
	: UID offset			{ $$.sym = $1; $$.ofs = $2; }
	| const				{ $$.sym = zerosym; $$.ofs = $1; }
	;

offset
	: '+' const			{ $$ = $2; }
	| '-' const			{ $$ = -$2; }
	| 				{ $$ = 0; }
	;
%% /***********************************************************************/

typedef struct FileStack {
  struct FileStack *prev;
  FILE *f;
  int line;
  char *name;
} FileStack;

FileStack *file_stack = 0;

FILE *infile;
FILE *outfile;

int scmp_a(void const *a, void const *b)
{
  return strcmp((*(Symbol **)a)->name, (*(Symbol **)b)->name);
}

int scmp_n(void const *a, void const *b)
{
  return (*(Symbol **)a)->value - (*(Symbol **)b)->value;
}

static int
opcode_compare (const void *e1, const void *e2)
{
  return strcmp (((struct opcode *)e1)->name, ((struct opcode *)e2)->name);
}

main(int argc, char **argv)
{
  Symbol *s;
  Patch *p;
  unsigned char exe[EXE_HEADER_SIZE+4];
  int undefs=0;
  int symcount = 0;
  int min_uninit;
  time_t now;
  char *outfilename, *leader;

  /* Sort the opcodes now so that we can use `bsearch' later.  */
  qsort (opcodes,
	 sizeof (opcodes) / sizeof (opcodes[0]),
	 sizeof (opcodes[0]),
	 opcode_compare);
  zerosym = set_symbol (get_symbol ("__zero__", 1), 0);

  if (argc < 2)
  {
    printf("usage: djasm infile [outfile] [mapfile]\n");
    exit(1);
  }
  inname = argv[1];
  infile = fopen(argv[1], "r");
  if (infile == 0)
  {
    fprintf(stderr, "Error: cannot open file %s for reading\n", argv[1]);
    perror("The error was");
    exit(1);
  }
  yyparse();
  fclose(infile);
  if (bsspc == -1)
  {
    bsspc = pc;
    generated_bytes = last_align_end==pc ? last_align_begin : pc;
  }

  sortsyms(scmp_a);

  for (s=symtab; s; s=s->next)
  {
    if (!istemp(s->name, 0))
      symcount++;
    if (!s->defined && s->patches)
    {
      fprintf(stderr, "Undefined symbol `%s', first used on line %d\n", s->name, s->first_used);
      for (p=s->patches; p; p=p->next)
        printf("\treferenced on line %d\n", p->lineno);
      undefs++;
    }
  }
  if (undefs)
    return 1;
  if (total_errors)
  {
    fprintf(stderr, "%d errors\n", total_errors);
    return 1;
  }

  printf("%#x bytes generated, %#x bytes in file, %#x bytes total, %d symbols\n",
    generated_bytes, bsspc, pc, symcount);

  min_uninit = (pc-bsspc+15)/16;

  memset(exe, 0, EXE_HEADER_SIZE+4);
  exe[0] = 0x4d;		/* MAGIC */
  exe[1] = 0x5a;
  exe[2] = bsspc;		/* bytes in last page */
  exe[3] = (bsspc>>8)&1;
  exe[4] = (bsspc+1023)>>9;	/* number of sectors */
  exe[5] = (bsspc+1023)>>17;
  exe[6] = 0;			/* relocation entries */
  exe[7] = 0;
  exe[8] = EXE_HEADER_BLOCKS;	/* blocks in header */
  exe[9] = 0;
  exe[10] = min_uninit;		/* min uninitialized paragraphs */
  exe[11] = (min_uninit>>8);
  exe[12] = 0xff;		/* max uninitialized paragraphs */
  exe[13] = 0xff;
  exe[14] = 0;			/* relative SS */
  exe[15] = 0;
  exe[16] = stack_ptr;		/* SP */
  exe[17] = stack_ptr>>8;
  exe[18] = 0;			/* checksum */
  exe[19] = 0;
  exe[20] = start_ptr;		/* IP */
  exe[21] = start_ptr >> 8;
  exe[22] = 0;			/* relative CS */
  exe[23] = 0;

  time(&now);
  sprintf(exe+24, "\r\n%s generated from %s by djasm, on %.24s\r\n", argv[2], argv[1], ctime(&now));
  if (copyright)
    strncat(exe+32, copyright, 480-strlen(exe+32));
  strcat(exe+32, "\r\n\032");

  if (argv[2] == 0)
  {
    char *dot=0, *sl=0, *cp;
    outfilename = (char *)malloc(strlen(argv[1])+5);
    strcpy(outfilename, argv[1]);
    for (cp=outfilename; *cp; cp++)
    {
      if (*cp == ':' || *cp == '\\' || *cp == '/')
      {
        sl = cp+1;
        dot = 0;
      }
      if (*cp == '.')
        dot = cp;
    }
    if (!dot)
    {
      dot = cp;
      *dot = '.';
    }
    strcpy(dot+1, ext_types[out_type]);
  }
  else
  {
    char *dot=0, *sl=0, *cp;
    outfilename = argv[2];
    for (cp=outfilename; *cp; cp++)
    {
      if (*cp == ':' || *cp == '\\' || *cp == '/')
      {
        sl = cp+1;
        dot = 0;
      }
      if (*cp == '.')
        dot = cp;
    }
    if (!dot)
    {
      sl = (char *)malloc(strlen(outfilename)+5);
      strcpy(sl, outfilename);
      outfilename = sl;
      dot = outfilename + strlen(outfilename);
      *dot = '.';
      strcpy(dot+1, ext_types[out_type]);
    }
    else
      set_out_type(dot+1);
  }

  switch (out_type)
  {
    case OUT_exe:
    case OUT_com:
    case OUT_bin:
      outfile = fopen(outfilename, "wb");
      break;
    case OUT_h:
    case OUT_inc:
    case OUT_s:
      outfile = fopen(outfilename, "w");
      break;
  }
  if (outfile == 0)
  {
    fprintf(stderr, "Error: cannot open file %s for writing\n", outfilename);
    perror("The error was");
    exit(1);
  }

  switch (out_type)
  {
    case OUT_exe:
      fwrite(exe, EXE_HEADER_SIZE, 1, outfile);
      fwrite(outbin, bsspc, 1, outfile);
      break;

    case OUT_com:
      fwrite(outbin+256, bsspc-256, 1, outfile);
      break;

    case OUT_bin:
      fwrite(outbin, bsspc, 1, outfile);
      break;

    case OUT_h:
      if (image_type == OUT_exe)
        for (i=0; i<EXE_HEADER_SIZE; i++)
        {
          fprintf(outfile, "0x%02x,", exe[i]);
          if ((i&15) == 15)
            fputc('\n', outfile);
        }
      for (i=((image_type==OUT_com)?0x100:0); i<bsspc; i++)
      {
        fprintf(outfile, "0x%02x", outbin[i]);
        if (i<bsspc-1)
          fputc(',', outfile);
        if ((i&15) == 15)
          fputc('\n', outfile);
      }
      if (i&15)
        fputc('\n', outfile);
      break;

    case OUT_inc:
    case OUT_s:
      if (out_type == OUT_inc)
        leader = INC_LEADER;
      else
        leader = S_LEADER;
      fputs(leader, outfile);
      if (image_type == OUT_exe)
        for (i=0; i<EXE_HEADER_SIZE; i++)
        {
          fprintf(outfile, "0x%02x", exe[i]);
          if ((i&15) == 15)
          {
            fputc('\n', outfile);
            fputs(leader, outfile);
          }
          else
            fputc(',', outfile);
        }
      for (i=((image_type==OUT_com)?0x100:0); i<bsspc; i++)
      {
        fprintf(outfile, "0x%02x", outbin[i]);
        if ((i&15) == 15)
        {
          fputc('\n', outfile);
          fputs(leader, outfile);
        }
        else
          if (i<bsspc-1)
            fputc(',', outfile);
      }
      if (i&15)
        fputc('\n', outfile);
      break;
  }
  fclose(outfile);
  
  if (argc > 3)
  {
    FILE *mapfile = fopen(argv[3], "w");
    fprintf(mapfile, "%#x bytes generated, %#x bytes in file, %#x bytes total, %d symbols\n",
      generated_bytes, bsspc, pc, symcount);

    fprintf(mapfile, "\nStart Stop  Length Name Class\n");
    fprintf(mapfile, "%04XH %04XH %04XH  code code\n", 0, pc-1, pc);

    fprintf(mapfile, "\nAddress    Symbols by Name\n\n");
    for (s = symtab; s; s=s->next)
      if (!istemp(s->name, 0))
        fprintf(mapfile, "0000:%04X  %s (%c)\n", s->value, s->name, SYMTYPES[s->type]);
    fprintf(mapfile, "\nAddress    Symbols by Value\n\n");
    sortsyms(scmp_n);
    for (s = symtab; s; s=s->next)
      if (!istemp(s->name, 0))
        fprintf(mapfile, "0000:%04X  %s (%c)\n", s->value, s->name, SYMTYPES[s->type]);
    fprintf(mapfile, "\nLine numbers for (%s)\n", argv[1]);
    for (i=0; i<num_lineaddr; i++)
    {
      fprintf(mapfile, "%5d 0000:%04X", lineaddr[i].line, lineaddr[i].addr);
      if ((i & 3) == 3)
        fputc('\n', mapfile);
      else
        fputc(' ', mapfile);
    }
    fputc('\n', mapfile);
    fclose(mapfile);
  }
}

void yyerror(char *s)
{
  fprintf(stderr, "%s:%d: %s\n", inname, lineno, s);
  strbuf[strbuflen] = 0;
  fprintf(stderr, "Last token was `%s' (%s)\n", last_token, yytname[yytranslate[last_tret]]);
  total_errors++;
}

Symbol *get_symbol(char *name, int create)
{
  Symbol *s;
  for (s=symtab; s; s=s->next)
    if (strcmp(name, s->name) == 0)
      return s;
  if (!create)
    return 0;
  s = (Symbol *)malloc(sizeof(Symbol));
  s->next = symtab;
  symtab = s;
  s->name = (char *)malloc(strlen(name)+1);
  strcpy(s->name, name);
  s->value = 0;
  s->defined = 0;
  s->patches = 0;
  s->first_used = lineno;
  s->type = SYM_unknown;
  return s;
}

Symbol *set_symbol(Symbol *s, int value)
{
  if (istemp(s->name, 'b'))
    s->defined = 0;
  if (s->defined)
    printf("Warning: symbol %s redefined\n", s->name);
  s->value = value;
  s->defined = 1;
  while (s->patches)
  {
    int v, o;
    unsigned char *cp;
    Patch *p = s->patches;
    s->patches = s->patches->next;
    switch (p->rel)
    {
    case REL_abs:
    case REL_abs32:
      v = value;
      break;
    case REL_16:
      v = value - p->location - 2;
      break;
    case REL_8:
      v = value - p->location - 1;
      break;
    }
    cp = outbin + p->location;
    switch (p->rel)
    {
    case REL_abs32:
      o = *((int *)cp);
      break;
    case REL_abs:
    case REL_16:
      o = (signed short)(cp[0] | (cp[1] << 8));
      break;
    case REL_8:
      o = (signed char)(cp[0]);
      break;
    }
    o += v;
    switch (p->rel)
    {
    case REL_abs32:
      *((int *)cp) = o;
      break;
    case REL_abs:
    case REL_16:
      cp[1] = o>>8;
      cp[0] = o;
      break;
    case REL_8:
      if (o > 127 || o < -128)
      {
	/* So far away from me
	   So far I just can't see
	   So far away from me
	   You're so far away from me
	   -- from `So far away' by Mark Knopfler.  */
	fprintf(stderr, "%s:%d: 8-bit relocation too big (%d); use long form\n", inname, p->lineno, o);
	total_errors++;
      }
      cp[0] = o;
      break;
    }
    free(p);
  }
  if (istemp(s->name, 'f'))
    s->defined = 0;
  return s;
}

void sortsyms(int (*sortf)(void const *,void const *))
{
  int ns, i;
  Symbol *s, **st;
  if (!symtab)
    return;
  for (s=symtab, ns=0; s; s=s->next)
    ns ++;
  st = (Symbol **)malloc(sizeof(Symbol *) * ns);
  for (s=symtab, ns=0; s; s=s->next, ns++)
    st[ns] = s;
  qsort(st, ns, sizeof(Symbol *), sortf);
  for (i=0; i<ns-1; i++)
    st[i]->next = st[i+1];
  st[i]->next = 0;
  symtab = st[0];
  free(st);
}

void emit(void *ptr, int len)
{
  while (pc + len > outsize)
  {
    outsize += 512;
    outbin = realloc(outbin, outsize);
  }
  set_lineaddr();
  memcpy(outbin+pc, ptr, len);
  pc += len;
}

void emitb(int b)
{
  emit(&b, 1);
}

void emitw(int w)
{
  emitb(w);
  emitb(w>>8);
}

void emitd(long d)
{
  emitw(d);
  emitw(d>>16);
}

void emits(Symbol *s, int offset, int rel)
{
  Patch *p;
  int v;
  if (s->defined)
  {
    switch (rel)
    {
    case REL_abs32:
      v = s->value + offset;
      emitd(v);
      break;
    case REL_abs:
      v = s->value + offset;
      emitw(v);
      break;
    case REL_16:
      v = s->value - pc - 2 + offset;
      emitw(v);
      break;
    case REL_8:
      v = s->value - pc - 1 + offset;
      if (v < -128 || v > 127)
      {
	fprintf(stderr, "%s:%d: 8-bit relocation too big (%d); use long form", inname, lineno, v);
	total_errors++;
      }
      emitb(v);
      break;
    }
    return;
  }
  p = (Patch *)malloc(sizeof(Patch));
  p->next = s->patches;
  s->patches = p;
  p->location = pc;
  p->lineno = lineno;
  p->rel = rel;
  switch (rel)
  {
  case REL_abs32:
    emitd(offset);
    break;
  case REL_abs:
  case REL_16:
    emitw(offset);
    break;
  case REL_8:
    if (offset < -128 || offset > 127)
    {
      fprintf(stderr, "%s:%d: 8-bit relocation offset too big (%d); use long form", inname, lineno, offset);
      total_errors++;
    }
    emitb(offset);
    break;
  }
}

void modrm(int mod, int reg, int rm)
{
  emitb((mod<<6) | (reg<<3) | rm);
}

int reg2mbyte[] = { 0x48, 0x88, 0x60, 0xa0, 0x40, 0x80, 0x20, 0x08 };

void reg(int which)
{
  int i;
  int v;
  int mbyte = which << 3;

  if (_modrm.regs == 0)
  {
    emitb(mbyte + 6);
  }
  else if (_modrm.regs == 0x20 && _modrm.offset == 0 && _modrm.nsyms == 0)
  {
    emitb(mbyte + 0x86);
  }
  else
  {
    for (i=0; i<8; i++)
      if (reg2mbyte[i] == _modrm.regs)
      {
        mbyte |= i;
        break;
      }
    if (i == 8)
    {
      printf("%s:%s: Invalid registers in R/M\n", inname, lineno);
      total_errors ++;
    }
    if (_modrm.offset == 0 && _modrm.nsyms == 0)
    {
      emitb(mbyte);
      _modrm.nsyms = _modrm.regs = _modrm.offset = 0;
      return;
    }
    emitb(mbyte + 0x80);
  }
  v = _modrm.offset;
  for (i=0; i<_modrm.nsyms; i++)
  {
    Symbol *s = _modrm.syms[i];
    if (s->defined)
      v += s->value;
    else
    {
      Patch *p;
      p = (Patch *)malloc(sizeof(Patch));
      p->next = s->patches;
      s->patches = p;
      p->location = pc;
      p->lineno = lineno;
      p->rel = REL_abs;
    }
  }   
  emitw(v);
  _modrm.nsyms = _modrm.regs = _modrm.offset = 0;
}

int yylex()
{
  last_tret = yylex1();
  return last_tret;
}

static struct {
  char c1, c2;
  int token;
} twochars[] = {
  '=', '=', '=',
  '>', '=', OP_GE,
  '<', '=', OP_LE,
  '<', '>', OP_NE,
  '!', '=', OP_NE,
  '&', '&', OP_AND,
  '|', '|', OP_OR,
  '<', '<', OP_SHL,
  '>', '>', OP_SHR};

int yylex1()
{
  int c, c2, i, oldc;
  struct opcode *opp, op;

  do {
    c = fgetc(infile);
  } while (c == ' ' || c == '\t');

  switch (c)
  {
    case EOF:
      if (file_stack)
      {
        FileStack *tmp = file_stack;
        fclose(infile);
        free(inname);
        lineno = file_stack->line + 1; /* Correct for .include line */
        infile = file_stack->f;
        inname = file_stack->name;
        file_stack = file_stack->prev;
        free(tmp);
        return yylex1();
      }
      return 0;

    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '_':
    case '$':
    case '.':
    case '@':
      ungetc(c, infile);
      fscanf(infile, "%[a-zA-Z0-9_$.@]", strbuf);
      strcpy(last_token, strbuf);
      if (strcmp(strbuf, ".") == 0)
        return PC;
      op.name = strbuf;
      opp = bsearch (&op,
		     opcodes, 
		     sizeof (opcodes) / sizeof (opcodes[0]),
		     sizeof (opcodes[0]),
		     opcode_compare);
      if (opp)
	{
	  yylval.i = opp->attr;
	  return opp->token;
	}
      else
	{
	  yylval.sym = get_symbol(strbuf,1);
	  return yylval.sym->defined ? KID : UID;
	}

    case '0' ... '9':
      ungetc(c, infile);
      fscanf(infile, "%i", &(yylval.i));
      sprintf(last_token, "%d", yylval.i);
      return NUMBER;
      break;

    case '>':
    case '<':
    case '!':
    case '&':
    case '|':
    case '=':
      c2 = fgetc (infile);
      for (i = 0; i < 9; i++)
	if (c == twochars[i].c1 && c2 == twochars[i].c2)
	  return twochars[i].token;
      ungetc (c2, infile);
      return c;

    case '"':
    case '\'':
      oldc = c;
      i = 0;
      while (1)
      {
        c = fgetc(infile);
        if (c == oldc)
        {
          strcpy(last_token, strbuf);
          strbuflen = i;
          if (strbuflen == 1)
          {
            yylval.i = strbuf[0];
            return NUMBER;
          }
          return STRING;
	}
        switch (c)
        {
          case '\\':
            switch (c = fgetc(infile))
            {
              case '0':
                strbuf[i++] = 0;
                break;
              case 'n':
                strbuf[i++] = '\n';
                break;
              case 'r':
                strbuf[i++] = '\r';
                break;
              case 't':
                strbuf[i++] = '\t';
                break;
              default:
                strbuf[i++] = c;
                break;
            }
            break;
          default:
            strbuf[i++] = c;
            break;
        }
      }
      abort ();
    case ';':
      while (fgetc(infile) != '\n');
      c = '\n';
      /* Fall through.  */
    case '\n':
      strcpy(last_token, "NL");
      return c;
    default:
      sprintf(last_token, "<%c>", c);
      return c;
  }
}

int istemp(char *symname, char which)
{
  if (symname[0] != '@') return 0;
  if (which)
    if (symname[1] != which) return 0;
  else
    if (symname[1] != 'f' && symname[1] != 'b') return 0;
  if (!isdigit(symname[2])) return 0;
  if (symname[3]) return 0;
  return 1;
}

void do_sreg_pop(int sreg)
{
  switch (sreg)
  {
    case 0: /* es */
      emitb(0x07);
      break;
    case 1: /* cs */
      yyerror("Cannot pop CS");
      break;
    case 2: /* ss */
      emitb(0x17);
      break;
    case 3: /* ds */
      emitb(0x1f);
      break;
    case 4: /* fs */
      emitb(0x0f);
      emitb(0xa1);
      break;
    case 5: /* gs */
      emitb(0x0f);
      emitb(0xa9);
      break;
  }
}

void do_sreg_push(int sreg)
{
  switch (sreg)
  {
    case 0: /* es */
      emitb(0x06);
      break;
    case 1: /* cs */
      emitb(0x0e);
      break;
    case 2: /* ss */
      emitb(0x16);
      break;
    case 3: /* ds */
      emitb(0x1e);
      break;
    case 4: /* fs */
      emitb(0x0f);
      emitb(0xa0);
      break;
    case 5: /* gs */
      emitb(0x0f);
      emitb(0xa8);
      break;
  }
}

void set_lineaddr()
{
  static int last_lineno = -1;
  if (lineno == last_lineno)
    return;
  last_lineno = lineno;
  if (num_lineaddr == max_lineaddr)
  {
    max_lineaddr += 32;
    if (lineaddr)
      lineaddr = (lineaddr_s *)realloc(lineaddr, max_lineaddr * sizeof(lineaddr_s));
    else
      lineaddr = (lineaddr_s *)malloc(max_lineaddr * sizeof(lineaddr_s));
  }
  lineaddr[num_lineaddr].line = lineno;
  lineaddr[num_lineaddr].addr = pc;
  num_lineaddr++;
}

void do_align(int p2)
{
  last_align_begin = pc;
  while ((pc+EXE_HEADER_SIZE) % p2)
    emitb(0x90);
  last_align_end = pc;
}

void add_copyright(char *buf)
{
  char *tmp;
  if (copyright == 0)
  {
    copyright = (char *)malloc(strlen(buf)+1);
    strcpy(copyright, buf);
    return;
  }
  tmp = (char *)malloc(strlen(copyright) + strlen(buf) + 3);
  strcpy(tmp, copyright);
  strcat(tmp, "\r\n");
  strcat(tmp, buf);
  free(copyright);
  copyright = tmp;
}

void add_rcs_ident(char *buf)
{
  char tmp[500];
  time_t now;
  struct tm *tm;
  time(&now);
  tm = localtime(&now);
  sprintf(tmp, "%cId: %s built %02d/%02d/%02d %02d:%02d:%02d by djasm $\n",
	  '$', inname,
	  tm->tm_mon + 1,
	  tm->tm_mday,
	  tm->tm_year,
	  tm->tm_hour,
	  tm->tm_min,
	  tm->tm_sec);
  add_copyright(tmp);
  sprintf(tmp, "@(#) %s built %02d/%02d/%02d %02d:%02d:%02d by djasm\n",
	  inname,
	  tm->tm_mon + 1,
	  tm->tm_mday,
	  tm->tm_year,
	  tm->tm_hour,
	  tm->tm_min,
	  tm->tm_sec);
  add_copyright(tmp);
}

void set_out_type(char *t)
{
  int i;
  for (i=0; ext_types[i]; i++)
  {
    if (strcmp(ext_types[i], t) == 0)
    {
      out_type = i;
      return;
    }
  }
  printf("Unknown output type: `%s'\n", t);
}

void set_image_type(char *t)
{
  int i;
  for (i=0; ext_types[i]; i++)
  {
    if (strcmp(ext_types[i], t) == 0)
    {
      if (i == OUT_com && image_type != OUT_com)
      {
        if (pc)
        {
          fprintf(stderr, "Cannot make com file without .type \"com\"\n");
          exit(1);
        }
        while (pc < 0x100)
          emitb(0x90);
      }
      image_type = i;
      return;
    }
  }
  printf("Unknown output type: `%s'\n", t);
}

void do_include(char *fname)
{
  FILE *f;
  FileStack *fs;
  
  f = fopen(fname, "r");
  if (!f)
  {
    fprintf(stderr, "Error: cannot open %s\n", fname);
    perror("The error was");
    return;
  }
  fs = (FileStack *)malloc(sizeof(FileStack));
  fs->line = lineno;
  fs->prev = file_stack;
  fs->f = infile;
  fs->name = inname;
  file_stack = fs;

  infile = f;
  inname = (char *)malloc(strlen(fname)+1);
  strcpy(inname, fname);
  lineno = 1;
}

/* #define DEBUG_RELOC */

void do_linkcoff (char *filename)
{
  long len;
  int f;
  char *data, *p;
  FILHDR *header;
  SCNHDR *f_thdr;		/* Text section header */
  SCNHDR *f_dhdr;		/* Data section header */
  SCNHDR *f_bhdr;		/* Bss section header */
  AOUTHDR f_ohdr;		/* Optional file header (a.out) */
  SYMENT *symbol;
  RELOC *rp;
  int cnt, i;
  void *base;
  int textbase, database, bssbase, delta;
  char smallname[9];

  f = open (filename, O_RDONLY | O_BINARY);
  if (f < 0)
    {
      fprintf (stderr, "Error: cannot open %s\n", filename);
      perror("The error was");
      return;
    }
  len = lseek (f, 0L, SEEK_END);
  lseek (f, 0L, SEEK_SET);
  data = alloca (len);
  read (f, data, (unsigned)len);
  close (f);

  header = (FILHDR *) data;
  f_thdr = (void *)data + sizeof (FILHDR) + header->f_opthdr;
  f_dhdr = f_thdr + 1;
  f_bhdr = f_dhdr + 1;
  if (I386BADMAG (*header)
      || header->f_nscns != 3
      || strcmp (f_thdr->s_name, _TEXT)
      || strcmp (f_dhdr->s_name, _DATA)
      || strcmp (f_bhdr->s_name, _BSS))
    {
      fprintf (stderr, "The file %s is not a valid COFF file.\n", filename);
      return;
    }

  textbase = pc;
  emit(data + f_thdr->s_scnptr, f_thdr->s_size);
  database = pc;
  emit(data + f_dhdr->s_scnptr, f_dhdr->s_size);
  bssbase = pc;
  for (i = 0; i < f_bhdr->s_size; i++)
    emitb (0);

#ifdef DEBUG_RELOC
  printf ("textbase is at %04x\n", textbase);
  printf ("database is at %04x\n", database);
  printf ("bssbase  is at %04x\n", bssbase);
#endif

  symbol = (void *) data + header->f_symptr;
  base = (void *) symbol + header->f_nsyms * SYMESZ;
  for (cnt = header->f_nsyms; cnt > 0; symbol++, cnt--)
    {
      if (symbol->e.e.e_zeroes == 0)
	p = base + symbol->e.e.e_offset;
      else
	strncpy (p = smallname, symbol->e.e_name, 8),
	p[8] = 0;

      switch (symbol->e_sclass)
	{
	case C_EXT:
	  switch (symbol->e_scnum)
	    {
	    case 1:
	      set_symbol (get_symbol (p, 1),
			  textbase + symbol->e_value)->type |= SYM_code;
	      break;
	    case 2:
	      set_symbol (get_symbol (p, 1),
			  textbase + symbol->e_value)->type |= SYM_data;
	      break;
	    case 3:
	      set_symbol (get_symbol (p, 1),
			  textbase + symbol->e_value)->type |= SYM_data;
	      break;
	    case N_UNDEF:
	      if (symbol->e_value == 0)
		0;  /* Nothing -- external reference.  */
	      else if (!get_symbol (p, 0))
		{
		  /* New common variable.  */
		  set_symbol (get_symbol (p, 1), pc)->type |= SYM_data;
		  for (i = 0; i < symbol->e_value; i++)
		    emitb (0);
		}
	      break;
	    }
	  break;
	}
      cnt -= symbol->e_numaux;
      symbol += symbol->e_numaux;
    }

  symbol = (void *) data + header->f_symptr;
  for (i = 0; i < 2; i++)
    {
      if (i == 0)
	rp = (RELOC *) (data + f_thdr->s_relptr),
	cnt = f_thdr->s_nreloc;
      else
	rp = (RELOC *) (data + f_dhdr->s_relptr),
	cnt = f_dhdr->s_nreloc;

      for (; cnt > 0; cnt--, rp++)
	{
	  Symbol *s;
	  int delta;

	  if (symbol[rp->r_symndx].e.e.e_zeroes == 0)
	    p = base + symbol[rp->r_symndx].e.e.e_offset;
	  else
	    strncpy (p = smallname, symbol[rp->r_symndx].e.e_name, 8),
	    p[8] = 0;

#ifdef DEBUG_RELOC
	  s = get_symbol (p, 0);
	  printf ("ofs=%04x  typ=%02x  sec=%d"
		  "  val=%08x  data=%08x"
		  "  name=%s\n",
		  rp->r_vaddr + textbase,
		  rp->r_type,
		  symbol[rp->r_symndx].e_scnum,
		  s ? s->value : 0, 
		  *((int *) (outbin + textbase + rp->r_vaddr)),
		  p);
#endif
	  if (!strcmp (p, _TEXT))
	    delta = textbase;
	  else if (!strcmp (p, _DATA))
	    delta = textbase;
	  else if (!strcmp (p, _BSS))
	    delta = textbase;
	  else
	    {
	      s = get_symbol (p, 1);
	      if (!s->defined)
		{
		  Patch *pat = (Patch *) malloc (sizeof (Patch));

		  if (rp->r_type == RELOC_REL32)
		    fprintf (stderr,
			     "Call from COFF file to (yet) undefined "
			     "destination, %s, is not supported.\n", p);
		  pat->next = s->patches;
		  s->patches = pat;
		  pat->location = textbase + rp->r_vaddr;
		  pat->lineno = -1;
		  pat->rel = REL_abs32;
		}
	    }

	  switch (rp->r_type)
	    {
	    case RELOC_ADDR32:
	      if (symbol[rp->r_symndx].e_scnum > 0)
		delta = textbase;
	      else
		delta = s->value - symbol[rp->r_symndx].e_value;
	      break;
	    case RELOC_REL32:
	      if (symbol[rp->r_symndx].e_scnum > 0)
		delta = 0;
	      else
		delta = s->value - textbase;
	      break;
	    default:
	      fprintf (stderr,
		       "COFF file %s contains bad relation "
		       "entry type (0x%02x).\n",
		       filename, rp->r_type);
	      delta = 0;
	    }
	  *((int *)(outbin + textbase + rp->r_vaddr)) += delta;
	}
    }
}

