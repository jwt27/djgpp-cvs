/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
void set_flag(char *name, char *new_value);
void clear_flag(char *name);
char *flag_value(char *name);
char *take_name(char *arg);
int ifset(char *line);
