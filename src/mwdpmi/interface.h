/* ---------------------------------------------------------------------- */
/* Copyright 1995-1996 by Morten Welinder (terra@diku.dk)                 */
/* Distributed under the GPL, see COPYING for details.                    */
/* ---------------------------------------------------------------------- */
/* Types...  */

typedef unsigned char bool;
typedef unsigned char word8;
typedef signed char int8;
typedef unsigned short word16;
typedef signed short int16;
typedef unsigned int word32;
typedef signed int int32;
/* ---------------------------------------------------------------------- */
/* Data from low-data.inc and immediates in low code -- const so nobody
   changes it in high memory.  */

extern const word8 cpu;
extern const bool xms_present;
extern const word16 code_seg;
extern const word32 page_dir_physical;
extern const word32 vcpi_entry;
extern struct pseudo_descriptor idt_rec;
extern const word16 unload_server_seg;
extern const word32 old15, old2f;
extern const word32 vcpi_descriptors;
extern word16 vcpi_memory_handle;
extern const word16 allocation_strategy;
extern const word16 umb_status;
extern word16 transfer_buffer_seg;
extern const word16 client_low_data_needs;
extern word32 gdt_low;
extern word16 go32_offset;
/* ---------------------------------------------------------------------- */
/* Data from highdata.inc -- consts to avoid changes to something that
   probably ought to be constant.  */

extern const word8 dos_major, dos_minor;
extern const bool vcpi_present;
extern const word8 memory_source;
extern const word32 low_code_linear;

extern const word16 memory_ext_first, memory_ext_last, server_ext_size;
extern const word8 server_stack, server_stack_start;
extern const word16 prefixseg;
extern word16 low_memory_end;
extern const word32 page_dir_linear;
extern const word32 server_page_table_linear;
extern const char code32_end;

extern const struct tss_segment vcpi_tss;  /* Moved.  */
/* ---------------------------------------------------------------------- */
/* Functions and the like.  */

extern void clean_exit (void);
extern void go32 (void);
extern void exit_server (void);
extern void unload_server (void);
extern void real_15 (void), real_2f (void);
/* ---------------------------------------------------------------------- */
