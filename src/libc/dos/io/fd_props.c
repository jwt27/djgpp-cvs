/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <limits.h>
#include <strings.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/stat.h>
#include <libc/bss.h>
#include <fcntl.h>
#include <io.h>
#include <unistd.h>
#include <libc/fd_props.h>

extern void (*__fd_properties_cleanup_hook);

static void open_fd(fd_properties *fd, int open_flags);
static void close_fd(fd_properties *fd);
static fd_properties * find_eq_filename(const fd_properties *fd);
static fd_properties * alloc_fd_properties(void);
static void free_fd_properties(fd_properties *);
static void insert_list(fd_properties **, fd_properties *);
static void remove_list(fd_properties **, fd_properties *);
static void exit_cleanup(void);

/* Array of pointers to fd_properties objects associated
   with a file descriptor.  */
fd_properties **__fd_properties = NULL;

/* List of fd_properties objects associated with at least
   one file descriptor.  */
static fd_properties *active_fds;

/* List of fd_properties objects not currently in use.  */
static fd_properties *cached_fds;

static int old_bss_count;

/* Perform file descriptor specific initialization.  */
static inline void
open_fd(fd_properties *cur_fd, int open_flags)
{
  cur_fd->flags = 0;
  if (cur_fd->filename && (open_flags & O_TEMPORARY))
    cur_fd->flags |= FILE_DESC_TEMPORARY;
  if (open_flags & O_APPEND)
    cur_fd->flags |= FILE_DESC_APPEND;
}

/* Perform file descriptor specific finalization.  */
static inline void
close_fd(fd_properties *cur_fd)
{
  /* Delete the file if no other descriptors use the file.  Otherwise,
     mark the other descriptor with the temporary flag.  */
  if (cur_fd->flags & FILE_DESC_TEMPORARY)
  {
    fd_properties *ptr = find_eq_filename(cur_fd);
    if (ptr)
      ptr->flags |= FILE_DESC_TEMPORARY;
    else
      remove(cur_fd->filename);
  }
}

/* Set properties associated with a file descriptor.  */
int
__set_fd_properties(int fd, const char *file, int open_flags)
{
  char *result;

  if (old_bss_count != __bss_count)
  {
    size_t size = 255 * sizeof(fd_properties *);
    old_bss_count = __bss_count;
    __fd_properties = malloc(size);
    active_fds = NULL;
    cached_fds = NULL;
    if (__fd_properties == NULL)
      return -1;
    memset(__fd_properties, 0, size);
    __fd_properties_cleanup_hook = exit_cleanup;
  }

  /* This function may be called twice for the same fd,
     so allocate and initialize when not already done.*/
  if (__fd_properties[fd] == NULL)
  {
    __fd_properties[fd] = alloc_fd_properties();
    if (__fd_properties[fd] == NULL)
      return -1;

    /* Initialize the object and insert it into list.  */
    __fd_properties[fd]->ref_count = 1;
    result = _truename(file, NULL);
    __fd_properties[fd]->filename = result;

    insert_list(&active_fds, __fd_properties[fd]);
  }

  open_fd(__fd_properties[fd], open_flags);

  return 0;
}

/* Set properties of a file descriptor returned by dup or dup2.  */
void
__dup_fd_properties(int from, int to)
{
  if (__fd_properties[from])
  {
    __fd_properties[to] = __fd_properties[from];
    ++(__fd_properties[to]->ref_count);
  }
}

/* Clear properties associated with a file descriptor.  */
int
__clear_fd_properties(int fd)
{
  /* If there are no properties with this descriptor then punt.  */
  if (__fd_properties[fd] == NULL)
    return -1;

  if (--(__fd_properties[fd]->ref_count) == 0)
  {
    /* The last file descriptor using this object has closed.  Perform
       any final actions before the object is put into the cache.  */
    close_fd(__fd_properties[fd]);

    free(__fd_properties[fd]->filename);
    __fd_properties[fd]->filename = NULL;
    free_fd_properties(__fd_properties[fd]);
  }
  __fd_properties[fd] = NULL;

  return 0;
}

/* Find another properties object using the same filename.  */
static
fd_properties *
find_eq_filename(const fd_properties *fd)
{
  fd_properties *ptr = active_fds;
  
  while (ptr)
  {
    if ((ptr != fd) && (stricmp(fd->filename, ptr->filename) == 0))
      return ptr;
    ptr = ptr->next;
  }
  return NULL;
}

/* Return a properties object for use with one or more file descriptors.  */
static fd_properties *
alloc_fd_properties(void)
{
  fd_properties *ptr;
  
  if (cached_fds == NULL)
  {
    ptr = malloc(sizeof(fd_properties));
    if (ptr == NULL)
      return ptr;
  }
  else
  {
    ptr = cached_fds;
    remove_list(&cached_fds, cached_fds);
  }
  ptr->prev = NULL;
  ptr->next = NULL;
  ptr->ref_count = 1;
  
  return ptr;   
}

/* Remove the object from the active list, and insert it into the cache.  */
static void
free_fd_properties(fd_properties *fd)
{
  remove_list(&active_fds, fd);
  insert_list(&cached_fds, fd);
}

/* Insert a properties object into a list.  */
static void
insert_list(fd_properties **head_ptr, fd_properties *item)
{
  fd_properties *head = *head_ptr;
  item->next = head;
  item->prev = NULL;
  if (head)
    head->prev = item;
  *head_ptr = item;
}

/* Remove a properties object from a list.  */
static void
remove_list(fd_properties **head_ptr, fd_properties *item)
{
  fd_properties *head = *head_ptr;
  
  if (item->prev)
  {
    (item->prev)->next = item->next;
    item->prev = NULL;
  }
  if (item->next)
  {
    (item->next)->prev = item->prev;
    item->next = NULL;
  }
  if (head == item)
    head = head->next;

  *head_ptr = head;
}

/* Close all known file descriptors to ensure all files marked
   as temporary are deleted at exit.  */
static void
exit_cleanup(void)
{
  int fd;

  if (__fd_properties == NULL || active_fds == NULL)
    return;

  fd = 0;
  while (fd < 255)
  {
    if (__fd_properties[fd])
      _close(fd);
    ++fd;
  }
}

