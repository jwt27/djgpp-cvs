#ifndef __dj_include_stub_h__
#define __dj_include_stub_h__

#define STUBINFO 0
#define STUBINFO_MAGIC 0
#define STUBINFO_SIZE 0x10
#define STUBINFO_MINSTACK 0x14
#define STUBINFO_MEMORY_HANDLE 0x18
#define STUBINFO_INITIAL_SIZE 0x1c
#define STUBINFO_MINKEEP 0x20
#define STUBINFO_DS_SELECTOR 0x22
#define STUBINFO_DS_SEGMENT 0x24
#define STUBINFO_PSP_SELECTOR 0x26
#define STUBINFO_CS_SELECTOR 0x28
#define STUBINFO_ENV_SIZE 0x2a
#define STUBINFO_BASENAME 0x2c
#define STUBINFO_ARGV0 0x34
#define STUBINFO_DPMI_SERVER 0x44
#define STUBINFO_STUBINFO_VER 0x54
#define STUBINFO_SELF_FD 0x58
#define STUBINFO_PAYLOAD_OFFS 0x5C
#define STUBINFO_PAYLOAD_SIZE 0x60
#define STUBINFO_END 0x64
#ifndef __ASSEMBLER__
typedef struct {
  char magic[16];
  unsigned long size;
  unsigned long minstack;
  unsigned long memory_handle;
  unsigned long initial_size;
  unsigned short minkeep;
  unsigned short ds_selector;
  unsigned short ds_segment;
  unsigned short psp_selector;
  unsigned short cs_selector;
  unsigned short env_size;
  char basename[8];
  char argv0[16];
  char dpmi_server[16];
  /* standard djgpp stubinfo ends here */
  unsigned long stubinfo_ver;
  long self_fd;
  unsigned long payload_offs;
  long payload_size;
} _GO32_StubInfo;
#endif

#endif /* __dj_include_stub_h__ */
