/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* zread.h -- common declarations for djtarx with decompression support
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

#ifndef __zread_h_
#define __zread_h_

#include <stdio.h>
#include <string.h>

#define memzero(s, n)   memset ((void *)(s), 0, (n))

#define local static

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

/* Return codes from djtarx */
#define OK      0
#define ERROR   1
#define WARNING 2

/* Compression methods */
#define STORED      0
#define COMPRESSED  1
#define PACKED      2
#define LZHED       3
/* methods 4 to 7 reserved */
#define DEFLATED    8
#define BZIP2ED     9
#define MAX_METHODS 10
extern int method;        /* compression method */

extern FILE *log_out;     /* the stream to output messages */

/* To save memory for 16 bit systems, some arrays are overlaid between
 * the various modules:
 * deflate:  prev+head   window      d_buf  l_buf  outbuf
 * unlzw:    tab_prefix  tab_suffix  stack  inbuf  outbuf
 * inflate:              window             inbuf
 * unpack:               window             inbuf  prefix_len
 * unlzh:    left+right  window      c_table inbuf c_len
 * For compression, input is done in window[]. For decompression, output
 * is done in window except for unlzw.
 */

#ifndef INBUFSIZ
# define INBUFSIZ  0x8000     /* input buffer size */
#endif
#define INBUF_EXTRA  64       /* required by unlzw() */

#ifndef OUTBUFSIZ
# define OUTBUFSIZ  16384     /* output buffer size */
#endif
#define OUTBUF_EXTRA 2048     /* required by unlzw() */

#ifndef DIST_BUFSIZE
# define DIST_BUFSIZE 0x8000  /* buffer for distances, see trees.c */
#endif

extern char inbuf[];
extern char outbuf[];         /* output buffer */
extern ush d_buf[];           /* buffer for distances, see trees.c */
extern uch window[];          /* Sliding window and suffix table (unlzw) */
#define tab_suffix window
#define tab_prefix prev       /* hash link (see deflate.c) */
#define head (prev + WSIZE)   /* hash head (see deflate.c) */
extern ush tab_prefix[];      /* prefix code (see unlzw.c) */

extern unsigned insize;       /* valid bytes in inbuf */
extern unsigned inptr;        /* index of next byte to be processed in inbuf */
extern unsigned outcnt;       /* bytes in output buffer */
extern long bytes_out;        /* number of bytes after decompression */
extern long header_bytes;     /* number of bytes in gzip header */
extern int part_nb;

extern void *ifd;             /* input file/diskette descriptor */
extern char *ifname;          /* input file name or "-" */
extern char *progname;        /* program name */
extern int  pkzip;            /* set for a pkzip file */

#define PACK_MAGIC     "\037\036"         /* Magic header for packed files */
#define GZIP_MAGIC     "\037\213"         /* Magic header for gzip files, 1F 8B */
#define OLD_GZIP_MAGIC "\037\236"         /* Magic header for gzip 0.5 = freeze 1.x */
#define LZH_MAGIC      "\037\240"         /* Magic header for SCO LZH Compress files*/
#define PKZIP_MAGIC    "\120\113\003\004" /* Magic header for pkzip files */
#define BZIP2_MAGIC    "\102\132\150"     /* Magic header for bzip2 files, BZh */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

/* internal file attribute */
#define UNKNOWN 0xffff
#define BINARY  0
#define ASCII   1

#ifndef WSIZE
# define WSIZE 0x8000         /* window size--must be a power of two, and */
#endif                        /* at least 32K for zip's deflate method */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

extern int v_switch;          /* be verbose (-v) */
extern int test;
extern int exit_code;         /* program exit code */
extern int z_switch;
extern int s_switch;          /* do not skip pax headers (-!s) */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf(0))
#define try_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf(1))

#define put_ubyte(c,f) {window[outcnt++] = (uch)(c); if (outcnt == WSIZE)\
   flush_window(f);}

/* Macros for getting two-byte and four-byte header values */
#define SH(p) ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8))
#define LG(p) ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16))

/* Diagnostic functions */
#ifdef DEBUG
# define Assert(cond,msg) {if(!(cond)) error(msg);}
# define Trace(x) fprintf x
# define Tracev(x) {if (v_switch) fprintf x ;}
# define Tracevv(x) {if (v_switch) fprintf x ;}
# define Tracec(c,x) {if (v_switch && (c)) fprintf x ;}
# define Tracecv(c,x) {if (v_switch && (c)) fprintf x ;}
#else
# define Assert(cond,msg)
# define Trace(x)
# define Tracev(x)
# define Tracevv(x)
# define Tracec(c,x)
# define Tracecv(c,x)
#endif

#define WARN(msg) {if (v_switch) fprintf msg ; \
                   if (exit_code == OK) exit_code = WARNING;}

int (*decompressor)(void *);

typedef enum { DOS_BINARY, DOS_TEXT, UNIX_TEXT } File_type;

        /* in djtar.c */
extern int change               (char *, const char *, int);
extern int isadir               (char *);
extern void do_directories      (char *);
extern File_type guess_file_type(char *, register size_t);
extern char* get_new_name       (char *, int *);
extern void make_directory      (char *);
extern void rename_if_dos_device(char *);

        /* in untar.c */
extern int tarread       (char *, long);
extern void tar_gz_read  (char *);

        /* in zmethod.c */
extern int get_method    (void *);

        /* in unzip.c */
extern int unzip         (void *);
extern int check_zipfile (void);

        /* in unpack.c */
extern int unpack        (void *);

        /* in unlzh.c */
extern int unlzh         (void *);

        /* in unbzip2.c */
extern int unbzip2       (void *);

        /* in util.c: */
extern int  copy         (void *);
extern ulg  updcrc       (uch *s, unsigned n);
extern void clear_bufs   (void);
extern int  fill_inbuf   (int eof_ok);
extern void flush_outbuf (void);
extern void flush_window (int (*)(char *, long));
extern void error        (const char *m);
extern void warn         (char *a, char *b);
extern void read_error   (void);
extern void *xmalloc     (unsigned int size);

        /* in inflate.c */
extern int inflate       (int (*)(char *, long));

        /* in epunzip.c */
extern void epunzip_read(char *);

#endif  /* __zread_h_ */
