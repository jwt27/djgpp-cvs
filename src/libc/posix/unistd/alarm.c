/* Copyright (C) 1995 Charles Sandmann (sandmann@clio.rice.edu)
   alarm() implmentation using setitimer 
   This software may be freely distributed, no warranty. */

#include <libc/stubs.h>
#include <unistd.h>
#include <sys/time.h>

unsigned int alarm(unsigned int seconds)
{
  struct itimerval new_values;

  new_values.it_value.tv_sec = seconds;
  new_values.it_value.tv_usec = 0;
  new_values.it_interval.tv_sec = new_values.it_interval.tv_usec = 0;

  setitimer(ITIMER_REAL, &new_values, NULL);
  return seconds;
}
