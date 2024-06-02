#ifndef STUB_H
#define STUB_H

#include "djdpmi.h"

struct dos_ops {
  unsigned (*_dos_open)(const char *pathname, unsigned mode, int *handle);
  unsigned (*_dos_read)(int handle, void *buffer, unsigned count, unsigned *numread);
  unsigned (*_dos_write)(int handle, const void *buffer, unsigned count, unsigned *numwrt);
  unsigned long (*_dos_seek)(int handle, unsigned long offset, int origin);
  int (*_dos_close)(int handle);
  int (*_dos_link_umb)(int on);
};

#define DD(r, n, a, ...) \
  r (*__##n) a;
#define DDv(r, n) \
  r (*__##n)(void);
#define vDD(n, a, ...) \
  void (*__##n) a;
#define vDDv(n) \
  void (*__##n)(void);

struct dpmi_ops {
#include "dpmi_inc.h"
};

#undef DD
#undef DDv
#undef vDD
#undef vDDv

struct stub_ret_regs {
  uint16_t fs;
  uint16_t ds;
  uint16_t cs;
  uint32_t eip;
};

int djstub_main(int argc, char *argv[], char *envp[], unsigned psp_sel,
    struct stub_ret_regs *regs, char *(*SEL_ADR)(uint16_t sel),
    struct dos_ops *dosops, struct dpmi_ops *dpmiops);

#endif
