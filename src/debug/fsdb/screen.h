/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
/* ------------------------------------------------------------------------- */
typedef enum { CL_Info, CL_Msg, CL_Error } CL_TYPE;

typedef struct MENU_ITEM {
  char *txt;
  void (* handler)(int);
  int info;
} MENU_ITEM;

typedef struct EDIT_ITEM {
  char *txt; 
  char *data;
} EDIT_ITEM;
/* ------------------------------------------------------------------------- */
int debug_screen_p;
int dual_monitor_p;
char *user_screen_save, *debug_screen_save;
unsigned char screen_attr;
unsigned char screen_attr_normal;
unsigned char screen_attr_source;
unsigned char screen_attr_focus;
unsigned char screen_attr_break;
unsigned char screen_attr_message;
unsigned char screen_attr_error;
unsigned char screen_attr_menu;
unsigned char screen_attr_menufocus;
unsigned char screen_attr_editframe;
unsigned char screen_attr_edittxt;
unsigned char screen_attr_editfield;
unsigned char screen_attr_editfocus;
int cols, rows;
int toplines, bottomlines;
char *read_buffer;
/* ------------------------------------------------------------------------- */
void put (int x, int y, char *txt);
void putl (int x, int y, int l, char *txt);
void draw (int x, int y, unsigned char ch, int delta, int count);
void highlight (int x, int y, int len);
void frame (int x1, int y1, int x2, int y2);
void put_screen (char *screen);
char *get_screen (void);
void debug_screen (void);
void user_screen (void);
void message (CL_TYPE class, char *fmt, ...);
int read_string (char *starttext);
void init_screen (void);
void init_colours (void);
void screen_mode (int);
int menu (char *, MENU_ITEM *, int *);
int edit (char *, EDIT_ITEM *, int);
void edit_colours (int);
