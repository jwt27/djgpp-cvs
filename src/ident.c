/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define mkid2(q,n) __##q##n
#define mkid(q,n) mkid2(q,n)

#define mkst2(q) #q
#define mkst(q) mkst2(q)

#define ident(i, n) \
char mkid(LIB, n) [] = i " DJGPP " mkst(LIB) " built " __DATE__ " " __TIME__ " by gcc " __VERSION__ " $";

ident("$Id:", _ident_string)

ident("@(#)", _sccs_ident)
