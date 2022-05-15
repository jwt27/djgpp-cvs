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
extern int debug_screen_p;
extern int dual_monitor_p;
extern char *user_screen_save, *debug_screen_save;
extern unsigned char screen_attr;
extern unsigned char screen_attr_normal;
extern unsigned char screen_attr_source;
extern unsigned char screen_attr_focus;
extern unsigned char screen_attr_break;
extern unsigned char screen_attr_message;
extern unsigned char screen_attr_error;
extern unsigned char screen_attr_menu;
extern unsigned char screen_attr_menufocus;
extern unsigned char screen_attr_editframe;
extern unsigned char screen_attr_edittxt;
extern unsigned char screen_attr_editfield;
extern unsigned char screen_attr_editfocus;
extern int cols, rows;
extern int toplines, bottomlines;
extern char *read_buffer;
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
