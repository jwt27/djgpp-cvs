/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
int	fileio_get(void);
void	fileio_unget(int);
void	fileio_include(char *);
char *	fileio_where(void);
void	fileio_queue(char *);
void	fileio_add_path(char *);
