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

static int
dtou(char *fname)
{
  int i, k, k2, sf, df, l, l2=0, err=0, isCR=0;
  char buf[16384];
  char tfname[FILENAME_MAX], *bn, *w;
  struct stat st;
  struct utimbuf tim1;
  sf = open(fname, O_RDONLY|O_BINARY);
  if (sf < 1)
  {
    perror(fname);
    return 1;
  }
  
  fstat (sf,&st);
  tim1.actime = st.st_atime;
  tim1.modtime = st.st_mtime;

  strcpy (tfname, fname);
  for (bn=w=tfname; *w; w++) 
    if (*w=='/' || *w=='\\' || *w==':') 
      bn = w+1;  
  if (bn) *bn=0;
  strcat (tfname,"utod.tm$");
  
  df = open(tfname, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0644);
  if (df < 1)
  {
    perror(tfname);
    close(sf);
    return 1;
  }

  k2=0;
  while ((l=read(sf, buf, 16384)) > 0)
  { 
    int CtrlZ=0;
    for (i=k=0; i<l; i++) 
      {
         if (isCR && buf[i]!=0x0A) buf[k++] = 0x0D; 
         if (buf[i]==0x0D) { isCR=1; continue; }
         if (buf[i]==0x1A) { CtrlZ=1; break; }
         	     else    buf[k++] = buf[i];
         isCR = 0;
      }
    l2=(k>0 ? write(df, buf, k) : 0);
    if (l2<0 || CtrlZ) break;
    if (l2!=k) { err=1; break; }
  }

  if (l<0) perror (fname);
  if (l2<0) perror (tfname);
  if (err) fprintf (stderr,"Cannot process file %s\n",fname);

  close(sf);
  close(df);

  if (l>=0 && l2>=0 && err==0)
  {
    remove(fname);
    rename(tfname, fname);
    utime(fname, &tim1);
    chown(fname, st.st_uid, st.st_gid);
    chmod(fname, st.st_mode);
  }
  else 
  {
    remove(tfname);
  }
  return 0;
}

int
main(int argc, char **argv)
{
  int rv = 0;
  for (argc--, argv++; argc; argc--, argv++)
    rv += dtou(*argv);
  return rv;
}

