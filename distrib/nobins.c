#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef __DJGPP__
#define stricmp strcasecmp
#endif

typedef struct Ext {
  struct Ext *next;
  char *ext;
} Ext;

char *known_ext[] = {
  "a",
  "com",
  "dll",
  "exe",
  "o",
  "obj",
  "tar",
  "taz",
  "tgz",
  "zip",
  0};


Ext *ext_list = 0;

int
main(int argc, char **argv)
{
  int i;
  char *ext;
  char line[256];

  for (i=1; i<argc; i++)
  {
    Ext *e = (Ext *)malloc(sizeof(Ext));
    e->ext = strdup(argv[i]);
    e->next = ext_list;
    ext_list = e;
  }

  while (scanf("%s", line) == 1)
  {
    int skip = 0;
    unsigned char buf[4];
    int fd;
    char *cp;

    fd = open(line, O_RDONLY|O_BINARY);
    if (fd < 0)
      continue;
    read(fd, buf, 4);
    close(fd);

    if (buf[0] == 0x4d && buf[1] == 0x5a) continue;
    if (buf[0] == 0x4c && buf[1] == 0x01) continue;
    if (buf[0] == 0x0b && buf[1] == 0x01) continue;
    if (buf[0] == 0x07 && buf[1] == 0x01) continue;
    if (buf[0] == 0x7f && buf[1] == 0x45
	&& buf[2] == 0x4c && buf[3] == 0x46) continue;

    ext = 0;
    for (cp=line; *cp; cp++)
    {
      if (*cp == '/' || *cp == ':' || *cp == '\\')
	ext = 0;
      if (*cp == '.')
	ext = cp+1;
    }
    if (ext)
    {
      Ext *e;
      int i;
      for (e=ext_list; e; e=e->next)
	if (stricmp(e->ext, ext) == 0)
	  break;
      if (e)
	continue;
      for (i=0; known_ext[i]; i++)
	if (stricmp(known_ext[i], ext) == 0)
	  break;
      if (known_ext[i])
	continue;
    }
    printf("%s\n", line);
  }
  return 0;
}
