/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* Modified by A.Pavenis to work also in different Unix clones */
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#if defined(MSDOS) || defined(_Windows)
#define IS_DIR_SEPARATOR(path) ((path) == '/' || (path) == '\\' || (path) == ':')
#else
#define IS_DIR_SEPARATOR(path) ((path) == '/')
#endif

#define IS_LAST_CR_IN_BUF  (i == l - 1)
#define IS_LAST_CR_IN_FILE (position + i + 1 == st.st_size)
#define SET_FLAG(flag)         \
do {                           \
  if ((flag) == 0) (flag) = 1; \
} while (0)
#define BUF_SIZE      16384

/* Control characters. */    
#define LF            0x0A
#define CR            0x0D
#define CntlZ         0x1A

/* Exit codes. */
#define NO_ERROR      0x00
#define IO_ERROR      0x01  /* Some I/O error occurred. */


static char *
BaseName (char * name)
{
  char * bn, *w;
  for (bn = w = name; *w; w++)
    if (IS_DIR_SEPARATOR (*w))
      bn = w+1;
  return bn;
}


static int
dtou(char *fname, int make_backup, int repair_mode, int strip_mode, int verbose, int vverbose, int preserve_timestamp)
{
  int i, k, sf, df, l, l2 = 0, is_CR = 0, is_nCR = 0, is_CR_sequence = 0;
  int CntlZ_flag = 0, CR_flag = 0, nCR_flag = 0, LF_flag = 0, exit_status = NO_ERROR;
  int buf_counter, nbufs, LF_counter, must_rewind, position, offset, whence;
  char buf[BUF_SIZE];
  char bfname[FILENAME_MAX], tfname[FILENAME_MAX], *bn;
  struct stat st;
  struct utimbuf tim1;

  stat (fname,&st);
  sf = open (fname, O_RDONLY|O_BINARY);
  if (sf < 1)
  {
    perror (fname);
    return IO_ERROR;
  }
  
  tim1.actime = st.st_atime;
  tim1.modtime = st.st_mtime;
  nbufs = st.st_size / BUF_SIZE;

  strcpy (tfname, fname);
  bn=BaseName(tfname);
  *bn=0;
  strcat (tfname,"dtou.tm$");
  if (make_backup)
  {
    strcpy (bfname, fname);
    if (pathconf ((fname), _PC_NAME_MAX) <= 12)
      for (i = strlen (bfname); i > -1; i--)
        if (bfname[i] == '.')
        {
          bfname[i] = '\0';
          break;
        }
    strcat (bfname,".d2u");
  }
  
  df = open (tfname, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0644);
  if (df < 1)
  {
    perror (tfname);
    close (sf);
    return IO_ERROR;
  }

  buf_counter = LF_counter = must_rewind = position = 0;
  if (strip_mode)
  {
    offset = 0;
    whence = SEEK_SET;
  }
  else
  {
    offset = -1;
    whence = SEEK_CUR;
  }
  while ((l = read (sf, buf, BUF_SIZE)) > 0)
  { 
    for (i = k = 0; i < l; i++) 
    {
      if (strip_mode)
      {
        if (buf[i] == LF)
        {
          if (!(is_CR || is_nCR)) SET_FLAG (LF_flag);
          if (is_nCR) { SET_FLAG (nCR_flag); is_nCR = 0; }
          if (is_CR) { SET_FLAG (CR_flag); is_CR = 0; }
          LF_counter++;
          offset = must_rewind = 0;
          buf[k++] = buf[i]; continue;
        }
        if (is_CR_sequence)
        {
          if (buf[i] == CR) { buf[k++] = buf[i]; continue; }
          else is_CR_sequence = 0;
        }
        if (is_nCR)
        {
          if (buf[i] != CR || IS_LAST_CR_IN_FILE)
          {
            is_CR_sequence = must_rewind = 1;
            is_nCR = 0; break;
          }
          else
            continue;
        }
        if (is_CR && buf[i] == CR) { is_nCR = 1; is_CR = 0; continue; }
        if (buf[i] == CR)
        {
          if (IS_LAST_CR_IN_FILE) { buf[k++] = buf[i]; break; }
          is_CR = must_rewind = 1;
          offset = position + i;
          continue;
        }
      }
      else
      {
        if (buf[i] == LF)
        {
          if (is_CR)  SET_FLAG (CR_flag);
          if (!is_CR) SET_FLAG (LF_flag);
          LF_counter++;
        }
        if (is_CR && buf[i] != LF) buf[k++] = CR;
        if (buf[i] == CR)
        {
          if (IS_LAST_CR_IN_BUF)
          {
            if (buf_counter < nbufs)
              must_rewind = 1;
            else
              buf[k++] = CR;
          }
          is_CR = 1; continue;
        }
        is_CR = 0;
      }

      if (!repair_mode)
        if (buf[i] == CntlZ) { SET_FLAG (CntlZ_flag); break; }

      buf[k++] = buf[i];
    }

    is_CR = 0;
    buf_counter++;
    position += l;
    if (must_rewind)
    {
      /* Last character/s in buf is/are CR/s.
         Push it/them back and reread it/them next time. */
      position = lseek (sf, offset, whence);
      must_rewind = 0;
    }

    l2 = (k > 0 ? write (df, buf, k) : 0);
    if (l2 < 0 || CntlZ_flag) break;
    if (l2 != k) { exit_status = IO_ERROR; break; }
  }

  if (l < 0) perror (fname);
  if (l2 < 0) perror (tfname);
  if (exit_status != NO_ERROR)
    fprintf (stderr,"Cannot process file %s\n",fname);

  close (sf);
  close (df);

  if (l >= 0 && l2 >= 0 && exit_status == NO_ERROR)
  {
    int file_has_changed = CR_flag || nCR_flag || CntlZ_flag || LF_flag;

    if (verbose)
      printf ("File: %s successfully processed.\n",fname);
    if (vverbose)
      printf ("File: %s\n",fname);

    if (CR_flag && vverbose) 
      printf ("At least one CR/LF to LF transformation occurred.\n");
    if (nCR_flag && vverbose) 
      printf ("Warning: At least one CR sequence stripped from a LF.\n");
    if (CntlZ_flag && vverbose) 
      printf ("Warning: At least one Cntl-Z has been found. File truncated at line %i.\n", LF_counter);
    if (LF_flag && vverbose) 
      printf ("Warning: At least one LF without a preceeding CR has been found.\n");

    if (vverbose && !file_has_changed)
      printf ("File unchanged.\n");

    if (make_backup && file_has_changed)
      rename (fname, bfname);
    else
      remove (fname);
    rename (tfname, fname);
    chown (fname, st.st_uid, st.st_gid);
    chmod (fname, st.st_mode);
    if (preserve_timestamp || !file_has_changed)
      utime (fname, &tim1);
  }
  else
  {
    remove (tfname);
    if (verbose || vverbose)
      printf ("File: %s. An I/O error occurred\n",fname);
  }

  return exit_status;
}

static void
usage(char *progname)
{
  printf ("Usage: %s [-b] [-h] [-r] [-s] [-t] [-v] [-vv] files...\n\n", progname);
  printf ("Options are:\n");
  printf ("            -b:  A backup of the original file is made using `.d2u' as backup\n");
  printf ("                 extension, if the file has been modified.\n");
  printf ("            -h:  Display this help and exit.\n");
  printf ("            -r:  Transform MSDOS-style EOF (CRLF) into UNIX-style EOL (LF).\n");
  printf ("                 Cntl-Z are ignored and will not truncate the file and\n");
  printf ("                 CR sequences in front of LF will be left unchanged.\n");
  printf ("            -s:  Transform MSDOS-style EOF (CRLF) into UNIX-style EOL (LF)\n");
  printf ("                 and strip a CR sequence of arbitrary length from the file,\n");
  printf ("                 if and only if the sequence is followed by LF. CR sequences\n");
  printf ("                 that are not followed by LF are always left unchanged.\n");
  printf ("            -t:  The timestamp of the modified file will not be preserved.\n");
  printf ("            -v:  Show if file processing has been successful or not.\n");
  printf ("           -vv:  Show the kind of modifications that have been done to the file.\n");
  printf ("The program is backward compatible with previous program versions if no options\n");
  printf ("are given at all. In this case, an occurrence of Cntl-Z will truncate the file,\n");
  printf ("MSDOS-style EOL (CRLF) are transformed into UNIX-style EOL (LF) and CR sequence\n");
  printf ("stripping will not happen at all. Also the timestamp will not be alterated and\n");
  printf ("no backup of the original file will be done.%s",
#if defined(__MSDOS__) || defined(_WIN32)
	  ""
#else
	  "\n"
#endif
	  );
}

int
main(int argc, char **argv)
{
  int exit_status = NO_ERROR, i, make_backup, repair_mode;
  int strip_mode, verbose, vverbose, preserve_timestamp;
  char* progname = BaseName(argv[0]);

  if (argc < 2)
  {
    usage (progname);
    exit(NO_ERROR);
  }

  /* Default for backward compatibility. */ 
  make_backup = repair_mode = strip_mode = verbose = vverbose = 0;
  preserve_timestamp = 1;

  i = 1;
  while ((argc > i) && (argv[i][0] == '-') && argv[i][1])
  {
    switch (argv[i][1])
    {
      case 'b':
        make_backup = 1;
        break;
      case 'h':
        usage (progname);
        exit(NO_ERROR);
        break;
      case 'r':
        repair_mode = 1;
        strip_mode = 0;
        break;
      case 's':
        strip_mode = 1;
        repair_mode = 0;
        break;
      case 't':
        preserve_timestamp = 0;
        break;
      case 'v':
        if (argv[i][2] == 'v')
        {
          vverbose = 1;
          verbose = 0;
        }
        else
        {
          verbose = 1;
          vverbose = 0;
        }
        break;
      default:
        fprintf (stderr, "%s: invalid option -- %s\n", progname, &argv[i][1]);
        fprintf (stderr, "Try `%s -h' for more information.\n", progname);
        exit (IO_ERROR);
        break;
    }
    i++;
  }

  for (; i < argc; i++)
    exit_status += dtou (argv[i], make_backup, repair_mode, strip_mode, verbose, vverbose, preserve_timestamp);
  return exit_status;
}
