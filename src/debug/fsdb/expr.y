/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
%{

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef TEST
#include "ed.h"
#include <debug/syms.h>
#endif
#define YYSTYPE long

int valid_addr(word32 addr, int len);

#ifdef TEST
unsigned long syms_name2val (char *);
int main (int, char **);
int undefined_symbol = 0;
#define valid_addr(x,y) 0
#define read_child (void)
#endif
int evaluate (char *, long *, char **);
static int yylex ();
static void yyerror (char *);

static long result;
static char *input;
static char *error;

%}

%token NUM

%left OP_LOR '|' '^'
%left OP_LAND '&'
%nonassoc OP_EQ '<' '>' OP_NE OP_LE OP_GE
%left OP_SHL OP_SHR
%left '+' '-'
%left '*' '/' '%'
%left ':'
%left OP_NEG OP_LNOT OP_NOT

%start main

%%

main
	: expr                          { result = $1; }
	;

expr
	: NUM				{ $$ = $1; }
	| expr OP_LOR expr		{ $$ = $1 || $3; }
	| expr OP_LAND expr		{ $$ = $1 && $3; }
	| expr '|' expr			{ $$ = $1 | $3; }
	| expr '&' expr			{ $$ = $1 & $3; }
	| expr '^' expr			{ $$ = $1 ^ $3; }
	| expr OP_EQ expr		{ $$ = $1 == $3; }
	| expr '>' expr			{ $$ = $1 > $3; }
	| expr '<' expr			{ $$ = $1 < $3; }
	| expr OP_GE expr		{ $$ = $1 >= $3; }
	| expr OP_LE expr		{ $$ = $1 <= $3; }
	| expr OP_NE expr		{ $$ = $1 != $3; }
	| expr OP_SHL expr		{ $$ = (unsigned)$1 << $3; }
	| expr OP_SHR expr		{ $$ = (unsigned)$1 >> $3; }
	| expr '+' expr			{ $$ = $1 + $3; }
	| expr '-' expr			{ $$ = $1 - $3; }
	| expr '*' expr			{ $$ = $1 * $3; }
	| expr '/' expr
	  { if ($3) $$ = $1 / $3; else yyerror ("Division by zero"); }
	| expr '%' expr
	  { if ($3) $$ = $1 % $3; else yyerror ("Division by zero"); }
	| '-' expr %prec OP_NEG		{ $$ = -$2; }
	| '!' expr %prec OP_LNOT	{ $$ = !$2; }
	| '~' expr %prec OP_NOT		{ $$ = ~$2; }
	| '(' expr ')'			{ $$ = $2; }
	| '[' expr ']'
	  { if (valid_addr ($2, 4))
	      { long l; read_child ($2, &l, 4); $$ = l; }
	    else
	      yyerror ("Error reading child");
	  }
	| expr ':' NUM
	  { switch ($3) {
	  case 1: $$ = ($1 & 0xff); break;
	  case 2: $$ = ($1 & 0xffff); break;
	  case 4: $$ = ($1 & 0xffffffff); break;
	  default: yyerror ("Invalid resizing");
	  }}
	;

%%

static int
yylex ()
{
  char *p, save;
  size_t i;
  static struct {
    char c1, c2;
    int token;
  } twochars[] = {
    { '=', '=', OP_EQ },
    { '>', '=', OP_GE },
    { '<', '=', OP_LE },
    { '!', '=', OP_NE },
    { '&', '&', OP_LAND},
    { '|', '|', OP_LOR },
    { '<', '<', OP_SHL},
    { '>', '>', OP_SHR}};

  while (isspace (*input)) input++;
  switch (*input)
    {
    case '0' ... '9':
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '%':
    case '_':
    case '#':
    case '.':
      p = input++;
      while (isalnum (*input)
	     || *input == '_' || *input == '#' || *input == '.')
	input++;
      save = *input;
      *input = 0;
      yylval = (long) syms_name2val (p);
      if (undefined_symbol)
	{
	  static char buf[80];
	  sprintf (buf, "Undefined symbol `%s'", p);
	  yyerror (buf);
	}
      *input = save;
      return NUM;
    case '\'':
      if (input[1] == '\\' && input[2] && input[3] == '\'')
	{
	  static char escaped[] = "e\et\tb\bn\nr\r\\\\''";
	  for (i = 0; escaped[i]; i += 2)
	    if (input[2] == escaped[i])
	      {
		input += 4;
		yylval = escaped[i + 1];
		return NUM;
	      }
	  yyerror ("Unrecognized escaped character");
	}
      else if (input[1] && input[2] == '\'')
	{
	  yylval = input[1];
	  input += 3;
	}
      else
	yyerror ("Unterminated character constant");
      return NUM;
    case 0:
      return 0;
    default:
      for (i = 0; i < sizeof (twochars) / sizeof (twochars[0]); i++)
	if (*input == twochars[i].c1 && input[1] == twochars[i].c2)
	  {
	    input += 2;
	    return twochars[i].token;
	  }
      return *input++;
    }
}

int
evaluate (char *exp, long *res, char **errtxt)
{
  int err;

  input = exp;
  error = 0;
  err = yyparse ();
  *res = result;
  *errtxt = error;
  undefined_symbol = 0;
  return err || error;
}

static void
yyerror (char *s)
{
  if (!error) error = s;
}


#ifdef TEST

unsigned long
syms_name2val (char *p)
{
  return strtoul(p, 0, 0);
}

int
main (int argc, char **argv)
{
  long res;
  int i, err;
  char *errtxt;

  for (i = 1; i < argc; i++)
    {
      err = evaluate (argv[i], &res, &errtxt);
      if (err)
	printf ("`%s' -> %s\n", argv[i], errtxt);
      else
	printf ("`%s' -> %ld\n", argv[i], res);
    }
  return 0;
}
#endif
