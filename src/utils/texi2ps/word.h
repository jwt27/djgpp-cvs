/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
void word_init(void);		/* called once at beginning */
void word_set_margins(int);	/* set margins */

void page_flush(void);		/* go to next page */
void page_flush_final(void);	/* go to next sheet */

void para_close(void);		/* close paragraph, start new one */
void para_set_prevailing_indent(int); /* indent (unit = points) */
void para_set_indent(int);	/* temporary relative indent (unit = points) */

void line_break(void);		/* force to beginning of next line if not already */
void line_skip(void);		/* skip a line */

void word_add_char(int c);	/* append character to current word */
void word_add_string(char *s);	/* append string to current word */
void word_add_quoted(int c);	/* like word_add_char, but no check for end of sentence */
void word_emit(void);		/* emit the word with no trailing whitespace */
void word_ws(void);		/* emit suitable whitespace */

void word_symbol(int sym);	/* emit a character from the symbol font */

void word_adjust_baseline(int pts);	/* y += pts */

extern int current_page;
extern int prevailing_indent;
extern int vskip_enabled;
extern int two_columns;
