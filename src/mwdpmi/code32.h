/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */

#include "defines.h"
#include "interface.h"
#include "cpu.h"

#define DPMI_OEM_VENDOR "Morten Welinder"
#define RING 3

#define MESSAGE(m) write (DOS_STDERR_FILENO, (m), strlen (m))

#define ENTRY MESSAGE ("Entering " __FUNCTION__ "\r\n")
#define EXIT  MESSAGE ("Leaving  " __FUNCTION__ "\r\n")

#define LINEAR_TO_PTR(l) ((void *)((l) - SERVER_LINEAR))
#define PTR_TO_LINEAR(p) ((word32)(p) + SERVER_LINEAR)

#ifdef DEBUG
#define DEBUG_TEST(flag) (debug_flags & (flag))
#define DEBUG_31		(1 << 0)
#define DEBUG_31_PAUSE		(1 << 1)
#define DEBUG_LDT		(1 << 4)
#define DEBUG_MEMORY		(1 << 5)
#define DEBUG_INT		(1 << 6)
#define DEBUG_CALLBACK		(1 << 7)
#else
#define DEBUG_TEST(flag) 0
#endif
/* ---------------------------------------------------------------------- */

/* libc.a */
#include <string.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/types.h>
#include <sys/movedata.h>
char *itoa (int, char *, int);
unsigned long strtoul (const char *, char **, int);

/* callback.c */
extern struct callback_stack callback_stack;
int callback_allocate (word16, word32, word16, word32, word16 *, word16 *);
int callback_free (word16 seg, word16 ofs);

/* clients.c */
int set_current_client (word16, int);
void client_free (void);
void client_terminate (int) __attribute__ ((noreturn));
int client_allocate_memory (word32, word32 *, word32 *, int);
int client_free_memory (word32);
int client_resize_memory (word32 *, word32 *, word32, int);
int client_memory_get_info (word32, word32 *, word32 *);

/* cmdline.c */
int parse_command_line (int);

/* globals.c */
#ifdef DEBUG
extern int debug_flags;
#endif
extern bool interrupts_safe;
extern bool one_shot_mode;
extern int client_count;
extern idt_entry *idt;
extern ptr3232 *current_software_interrupt_table;
extern char *argv0;
extern word16 current_client_handle;
extern struct client_low_data *current_client_low;
extern struct client_high_data *current_client;
extern word16 primary_client_handle;
extern __dpmi_regs termination_regs;
extern bool special_transfer_buffer;

/* interrupt.S */
void interrupt_entries (void);
void reflect (void);
void prot_21_entry (void);
void prot_2f_entry (void);
void prot_31_entry (void);

/* ldt.c */
int ldt_allocate (int);
#ifdef DEBUG
void ldt_print (void);
#endif

/* main.c */
void shutdown (int) __attribute__ ((noreturn));
void tsr (void) __attribute__ ((noreturn));
void unload (int) __attribute__ ((noreturn));
void init_server32 (void) __attribute__ ((noreturn));
void parse_extra_cmdline (void) __attribute__ ((noreturn));
int unload_safe (void);

/* memory.c */
void map_page_table (word32, word32);
void malloc_init (void);
void malloc_shutdown (void);
int palloc (void);
void pfree (int);
void free (void *);
void *malloc (int);
void *xmalloc (int);
word32 ptr_to_physical (void *);
word32 linear_alloc (word32);
void linear_free (word32, word32);
int commit_page (word32 *);
int uncommit_page (word32 *);
void physical_poke (word32, word32);
void memory_info (int *free_pages);

/* misc.c */
void callback_entry (void);

/* prot-31.c */
void prot_31 ( /* Lots o' regs */ );

/* serv-inc.c */
void server_goto (int, __dpmi_regs *);
void server_int (int, __dpmi_regs *);
void server_jump (int, int, __dpmi_regs *) __attribute__ ((noreturn));
void server_call (int, int, __dpmi_regs *);

/* sprintf.c */
void sprintf (char *, const char *, ...)
     __attribute__ ((format (printf, 2, 3)));
void printf (const char *, ...) __attribute__ ((format (printf, 1, 2)));
void eprintf (const char *, ...) __attribute__ ((format (printf, 1, 2)));

/* write.c */
int write (int, const void *, int);
