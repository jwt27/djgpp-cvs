/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/*
 * D_STRERR.C.
 *
 * Written by Peter J. Farley III 2001 <pjfarley@banet.net>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dos.h>
#include <libc/unconst.h>

#define RESERVED "Reserved"

/* Values for extended error code (EXTERROR field): */
const char *__dos_errlist [] = {
/* ---DOS 2.0+ --- */
/* 00h (0)   */ "No error",
/* 01h (1)   */ "Function number invalid",
/* 02h (2)   */ "File not found",
/* 03h (3)   */ "Path not found",
/* 04h (4)   */ "Too many open files (no handles available)",
/* 05h (5)   */ "Access denied",
/* 06h (6)   */ "Invalid handle",
/* 07h (7)   */ "Memory control block destroyed",
/* 08h (8)   */ "Insufficient memory",
/* 09h (9)   */ "Memory block address invalid",
/* 0Ah (10)  */ "Environment invalid (usually >32K in length)",
/* 0Bh (11)  */ "Format invalid",
/* 0Ch (12)  */ "Access code invalid",
/* 0Dh (13)  */ "Data invalid",
/* 0Eh (14)  */ "Reserved/(PTS-DOS 6.51+, S/DOS 1.0+) fixup overflow",
/* 0Fh (15)  */ "Invalid drive",
/* 10h (16)  */ "Attempted to remove current directory",
/* 11h (17)  */ "Not same device",
/* 12h (18)  */ "No more files",
/* ---DOS 3.0+ (INT 24 errors)--- */
/* 13h (19)  */ "Disk write-protected",
/* 14h (20)  */ "Unknown unit",
/* 15h (21)  */ "Drive not ready",
/* 16h (22)  */ "Unknown command",
/* 17h (23)  */ "Data error (CRC)",
/* 18h (24)  */ "Bad request structure length",
/* 19h (25)  */ "Seek error",
/* 1Ah (26)  */ "Unknown media type (non-DOS disk)",
/* 1Bh (27)  */ "Sector not found",
/* 1Ch (28)  */ "Printer out of paper",
/* 1Dh (29)  */ "Write fault",
/* 1Eh (30)  */ "Read fault",
/* 1Fh (31)  */ "General failure",
/* 20h (32)  */ "Sharing violation",
/* 21h (33)  */ "Lock violation",
/* 22h (34)  */ "Disk change invalid (ES:DI -> media ID structure)(see #01681)",
/* 23h (35)  */ "FCB unavailable/(PTS-DOS 6.51+, S/DOS 1.0+) Bad FAT",
/* 24h (36)  */ "Sharing buffer overflow",
/* 25h (37)  */ "(DOS 4.0+) Code page mismatch",
/* 26h (38)  */ "(DOS 4.0+) Cannot complete file operation (EOF / out of input)",
/* 27h (39)  */ "(DOS 4.0+) Insufficient disk space",
/* 28h (40)  */ RESERVED,
/* 29h (41)  */ RESERVED,
/* 2Ah (42)  */ RESERVED,
/* 2Bh (43)  */ RESERVED,
/* 2Ch (44)  */ RESERVED,
/* 2Dh (45)  */ RESERVED,
/* 2Eh (46)  */ RESERVED,
/* 2Fh (47)  */ RESERVED,
/* 30h (48)  */ RESERVED,
/* 31h (49)  */ RESERVED,
/* ---OEM network errors (INT 24)--- */
/* 32h (50)  */ "Network request not supported",
/* 33h (51)  */ "Remote computer not listening",
/* 34h (52)  */ "Duplicate name on network",
/* 35h (53)  */ "Network name not found",
/* 36h (54)  */ "Network busy",
/* 37h (55)  */ "Network device no longer exists",
/* 38h (56)  */ "Network BIOS command limit exceeded",
/* 39h (57)  */ "Network adapter hardware error",
/* 3Ah (58)  */ "Incorrect response from network",
/* 3Bh (59)  */ "Unexpected network error",
/* 3Ch (60)  */ "Incompatible remote adapter",
/* 3Dh (61)  */ "Print queue full",
/* 3Eh (62)  */ "Queue not full",
/* 3Fh (63)  */ "Not enough space to print file",
/* 40h (64)  */ "Network name was deleted",
/* 41h (65)  */ "Network: Access denied",
/*       (DOS 3.0+ [maybe 3.3+???]) codepage switching not possible
           (see also INT 21/AX=6602h,INT 2F/AX=AD42h) */
/* 42h (66)  */ "Network device type incorrect",
/* 43h (67)  */ "Network name not found",
/* 44h (68)  */ "Network name limit exceeded",
/* 45h (69)  */ "Network BIOS session limit exceeded",
/* 46h (70)  */ "Temporarily paused",
/* 47h (71)  */ "Network request not accepted",
/* 48h (72)  */ "Network print/disk redirection paused",
/* 49h (73)  */ "Network software not installed/(LANtastic) Invalid network version",
/* 4Ah (74)  */ "Unexpected adapter close/(LANtastic) Account expired",
/* 4Bh (75)  */ "(LANtastic) Password expired",
/* 4Ch (76)  */ "(LANtastic) Login attempt invalid at this time",
/* 4Dh (77)  */ "(LANtastic v3+) Disk limit exceeded on network node",
/* 4Eh (78)  */ "(LANtastic v3+) Not logged in to network node",
/* 4Fh (79)  */ RESERVED,
/* ---end of errors reportable via INT 24--- */
/* 50h (80)  */ "File exists",
/* 51h (81)  */ "(undoc) Duplicated FCB",
/* 52h (82)  */ "Cannot make directory",
/* 53h (83)  */ "Fail on INT 24h",
/* ---network-related errors (non-INT 24)--- */
/* 54h (84)  */ "(DOS 3.3+) Too many redirections / out of structures",
/* 55h (85)  */ "(DOS 3.3+) Duplicate redirection / already assigned",
/* 56h (86)  */ "(DOS 3.3+) Invalid password",
/* 57h (87)  */ "(DOS 3.3+) Invalid parameter",
/* 58h (88)  */ "(DOS 3.3+) Network write fault",
/* 59h (89)  */ "(DOS 4.0+) Function not supported on network / no process slots available",
/* 5Ah (90)  */ "(DOS 4.0+) Required system component not installed / not frozen",
/* 5Bh (91)  */ "(DOS 4.0+,NetWare4) Timer server table overflowed",
/* 5Ch (92)  */ "(DOS 4.0+,NetWare4) Duplicate in timer service table",
/* 5Dh (93)  */ "(DOS 4.0+,NetWare4) No items to work on",
/* 5Eh (94)  */ RESERVED,
/* 5Fh (95)  */ "(DOS 4.0+,NetWare4) Interrupted / invalid system call",
/* 60h (96)  */ RESERVED,
/* 61h (97)  */ RESERVED,
/* 62h (98)  */ RESERVED,
/* 63h (99)  */ RESERVED,
/* 64h (100) */ "(MSCDEX) Unknown error/(DOS 4.0+,NetWare4) Open semaphore limit exceeded",
/* 65h (101) */ "(MSCDEX) Not ready/(DOS 4.0+,NetWare4) Exclusive semaphore is already owned",
/* 66h (102) */ "(MSCDEX) EMS memory no longer valid/(DOS 4.0+,NetWare4) Semaphore was set when close attempted",
/* 67h (103) */ "(MSCDEX) Not High Sierra or ISO-9660 format/(DOS 4.0+,NetWare4) Too many exclusive semaphore requests",
/* 68h (104) */ "(MSCDEX) Door open/(DOS 4.0+,NetWare4) Operation invalid from interrupt handler",
/* 69h (105) */ "(DOS 4.0+,NetWare4) Semaphore owner died",
/* 6Ah (106) */ "(DOS 4.0+,NetWare4) Semaphore limit exceeded",
/* 6Bh (107) */ "(DOS 4.0+,NetWare4) Insert drive B: disk into A: / disk changed",
/* 6Ch (108) */ "(DOS 4.0+,NetWare4) Drive locked by another process",
/* 6Dh (109) */ "(DOS 4.0+,NetWare4) Broken pipe",
/* 6Eh (110) */ "(DOS 5.0+,NetWare4) Pipe open/create failed",
/* 6Fh (111) */ "(DOS 5.0+,NetWare4) Pipe buffer overflowed",
/* 70h (112) */ "(DOS 5.0+,NetWare4) Disk full",
/* 71h (113) */ "(DOS 5.0+,NetWare4) No more search handles",
/* 72h (114) */ "(DOS 5.0+,NetWare4) Invalid target handle for dup2",
/* 73h (115) */ "(DOS 5.0+,NetWare4) Bad user virtual address / protection violation",
/* 74h (116) */ "(DOS 5.0+) VIOKBD request/(NetWare4) error on console I/O",
/* 75h (117) */ "(DOS 5.0+,NetWare4) Unknown category code for IOCTL",
/* 76h (118) */ "(DOS 5.0+,NetWare4) Invalid value for verify flag",
/* 77h (119) */ "(DOS 5.0+,NetWare4) Level four driver not found by DOS IOCTL",
/* 78h (120) */ "(DOS 5.0+,NetWare4) Invalid / unimplemented function number",
/* 79h (121) */ "(DOS 5.0+,NetWare4) Semaphore timeout",
/* 7Ah (122) */ "(DOS 5.0+,NetWare4) Buffer too small to hold return data",
/* 7Bh (123) */ "(DOS 5.0+,NetWare4) Invalid character or bad file-system name",
/* 7Ch (124) */ "(DOS 5.0+,NetWare4) Unimplemented information level",
/* 7Dh (125) */ "(DOS 5.0+,NetWare4) No volume label found",
/* 7Eh (126) */ "(DOS 5.0+,NetWare4) Module handle not found",
/* 7Fh (127) */ "(DOS 5.0+,NetWare4) Procedure address not found",
/* 80h (128) */ "(DOS 5.0+,NetWare4) CWait found no children",
/* 81h (129) */ "(DOS 5.0+,NetWare4) CWait children still running",
/* 82h (130) */ "(DOS 5.0+,NetWare4) Invalid operation for direct disk-access handle",
/* 83h (131) */ "(DOS 5.0+,NetWare4) Attempted seek to negative offset",
/* 84h (132) */ "(DOS 5.0+,NetWare4) Attempted to seek on device or pipe",
/* ---JOIN/SUBST Errors--- */
/* 85h (133) */ "(DOS 5.0+,NetWare4) Drive already has JOINed drives",
/* 86h (134) */ "(DOS 5.0+,NetWare4) Drive is already JOINed",
/* 87h (135) */ "(DOS 5.0+,NetWare4) Drive is already SUBSTed",
/* 88h (136) */ "(DOS 5.0+,NetWare4) Can not delete drive which is not JOINed",
/* 89h (137) */ "(DOS 5.0+,NetWare4) Can not delete drive which is not SUBSTed",
/* 8Ah (138) */ "(DOS 5.0+,NetWare4) Can not JOIN to a JOINed drive",
/* 8Bh (139) */ "(DOS 5.0+,NetWare4) Can not SUBST to a SUBSTed drive",
/* 8Ch (140) */ "(DOS 5.0+,NetWare4) Can not JOIN to a SUBSTed drive",
/* 8Dh (141) */ "(DOS 5.0+,NetWare4) Can not SUBST to a JOINed drive",
/* 8Eh (142) */ "(DOS 5.0+,NetWare4) Drive is busy",
/* 8Fh (143) */ "(DOS 5.0+,NetWare4) Can not JOIN/SUBST to same drive",
/* 90h (144) */ "(DOS 5.0+,NetWare4) Directory must not be root directory",
/* 91h (145) */ "(DOS 5.0+,NetWare4) Can only JOIN to empty directory",
/* 92h (146) */ "(DOS 5.0+,NetWare4) Path is already in use for SUBST",
/* 93h (147) */ "(DOS 5.0+,NetWare4) Path is already in use for JOIN",
/* 94h (148) */ "(DOS 5.0+,NetWare4) Path is in use by another process",
/* 95h (149) */ "(DOS 5.0+,NetWare4) Directory previously SUBSTituted",
/* 96h (150) */ "(DOS 5.0+,NetWare4) System trace error",
/* 97h (151) */ "(DOS 5.0+,NetWare4) Invalid event count for DosMuxSemWait",
/* 98h (152) */ "(DOS 5.0+,NetWare4) Too many waiting on mutex",
/* 99h (153) */ "(DOS 5.0+,NetWare4) Invalid list format",
/* 9Ah (154) */ "(DOS 5.0+,NetWare4) Volume label too large",
/* 9Bh (155) */ "(DOS 5.0+,NetWare4) Unable to create another TCB",
/* 9Ch (156) */ "(DOS 5.0+,NetWare4) Signal refused",
/* 9Dh (157) */ "(DOS 5.0+,NetWare4) Segment discarded",
/* 9Eh (158) */ "(DOS 5.0+,NetWare4) Segment not locked",
/* 9Fh (159) */ "(DOS 5.0+,NetWare4) Invalid thread-ID address",
/* ----- */
/* A0h (160) */ "(DOS 5.0+) Bad arguments/(NetWare4) bad environment pointer",
/* A1h (161) */ "(DOS 5.0+,NetWare4) Invalid pathname passed to EXEC",
/* A2h (162) */ "(DOS 5.0+,NetWare4) Signal already pending",
/* A3h (163) */ "(DOS 5.0+) Uncertain media",
/* A3h (163) */ "(NetWare4) ERROR_124 mapping",
/* A4h (164) */ "(DOS 5.0+) Maximum number of threads reached/(NetWare4) No more process slots",
/* A5h (165) */ "(NetWare4) ERROR_124 mapping",
/* A6h (166) */ RESERVED,
/* A7h (167) */ RESERVED,
/* A8h (168) */ RESERVED,
/* A9h (169) */ RESERVED,
/* AAh (170) */ RESERVED,
/* ABh (171) */ RESERVED,
/* ACh (172) */ RESERVED,
/* ADh (173) */ RESERVED,
/* AEh (174) */ RESERVED,
/* AFh (175) */ RESERVED,
/* B0h (176) */ "(MS-DOS 7.0) Volume is not locked",
/* B1h (177) */ "(MS-DOS 7.0) Volume is locked in drive",
/* B2h (178) */ "(MS-DOS 7.0) Volume is not removable",
/* B4h (180) */ "(MS-DOS 7.0) Lock count has been exceeded/(NetWare4) Invalid segment number",
/* B5h (181) */ "(MS-DOS 7.0) A valid eject request failed/(DOS 5.0-6.0,NetWare4) Invalid call gate",
/* B6h (182) */ "(DOS 5.0+,NetWare4) Invalid ordinal",
/* B7h (183) */ "(DOS 5.0+,NetWare4) Shared segment already exists",
/* B8h (184) */ "(DOS 5.0+,NetWare4) No child process to wait for",
/* B9h (185) */ "(DOS 5.0+,NetWare4) NoWait specified and child still running",
/* BAh (186) */ "(DOS 5.0+,NetWare4) Invalid flag number",
/* BBh (187) */ "(DOS 5.0+,NetWare4) Semaphore does not exist",
/* BCh (188) */ "(DOS 5.0+,NetWare4) Invalid starting code segment",
/* BDh (189) */ "(DOS 5.0+,NetWare4) Invalid stack segment",
/* BEh (190) */ "(DOS 5.0+,NetWare4) Invalid module type (DLL can not be used as application)",
/* BFh (191) */ "(DOS 5.0+,NetWare4) Invalid EXE signature",
/* C0h (192) */ "(DOS 5.0+,NetWare4) EXE marked invalid",
/* C1h (193) */ "(DOS 5.0+,NetWare4) Bad EXE format (e.g. DOS-mode program)",
/* C2h (194) */ "(DOS 5.0+,NetWare4) Iterated data exceeds 64K",
/* C3h (195) */ "(DOS 5.0+,NetWare4) Invalid minimum allocation size",
/* C4h (196) */ "(DOS 5.0+,NetWare4) Dynamic link from invalid Ring",
/* C5h (197) */ "(DOS 5.0+,NetWare4) IOPL not enabled",
/* C6h (198) */ "(DOS 5.0+,NetWare4) Invalid segment descriptor privilege level",
/* C7h (199) */ "(DOS 5.0+,NetWare4) Automatic data segment exceeds 64K",
/* C8h (200) */ "(DOS 5.0+,NetWare4) Ring2 segment must be moveable",
/* C9h (201) */ "(DOS 5.0+,NetWare4) Relocation chain exceeds segment limit",
/* CAh (202) */ "(DOS 5.0+,NetWare4) Infinite loop in relocation chain",
/* CBh (203) */ "(NetWare4) Environment variable not found",
/* CCh (204) */ "(NetWare4) Not current country",
/* CDh (205) */ "(NetWare4) No signal sent",
/* CEh (206) */ "(NetWare4) File name not 8.3",
/* CFh (207) */ "(NetWare4) Ring2 stack in use",
/* D0h (208) */ "(NetWare4) Meta expansion is too long",
/* D1h (209) */ "(NetWare4) Invalid signal number",
/* D2h (210) */ "(NetWare4) Inactive thread",
/* D3h (211) */ "(NetWare4) File system information not available",
/* D4h (212) */ "(NetWare4) Locked error",
/* D5h (213) */ "(NetWare4) Attempted to execute non-family API call in DOS mode",
/* D6h (214) */ "(NetWare4) Too many modules",
/* D7h (215) */ "(NetWare4) Nesting not allowed",
/* D8h (216) */ RESERVED,
/* D9h (217) */ RESERVED,
/* DAh (218) */ RESERVED,
/* DBh (219) */ RESERVED,
/* DCh (220) */ RESERVED,
/* DDh (221) */ RESERVED,
/* DEh (222) */ RESERVED,
/* DFh (223) */ RESERVED,
/* E0h (224) */ RESERVED,
/* E1h (225) */ RESERVED,
/* E2h (226) */ RESERVED,
/* E3h (227) */ RESERVED,
/* E4h (228) */ RESERVED,
/* E5h (229) */ RESERVED,
/* E6h (230) */ "(NetWare4) Non-existent pipe, or bad operation",
/* E7h (231) */ "(NetWare4) Pipe is busy",
/* E8h (232) */ "(NetWare4) No data available for nonblocking read",
/* E9h (233) */ "(NetWare4) Pipe disconnected by server",
/* EAh (234) */ "(NetWare4) More data available",
/* EBh (235) */ RESERVED,
/* ECh (236) */ RESERVED,
/* EDh (237) */ RESERVED,
/* EEh (238) */ RESERVED,
/* EFh (239) */ RESERVED,
/* F0h (240) */ RESERVED,
/* F1h (241) */ RESERVED,
/* F2h (242) */ RESERVED,
/* F3h (243) */ RESERVED,
/* F4h (244) */ RESERVED,
/* F5h (245) */ RESERVED,
/* F6h (246) */ RESERVED,
/* F7h (247) */ RESERVED,
/* F8h (248) */ RESERVED,
/* F9h (249) */ RESERVED,
/* FAh (250) */ RESERVED,
/* FBh (251) */ RESERVED,
/* FCh (252) */ RESERVED,
/* FDh (253) */ RESERVED,
/* FEh (254) */ RESERVED,
/* FFh (255) */ "(NetWare4) Invalid drive"
/* (End of list) */ };

int __dos_nerr = sizeof(__dos_errlist) / sizeof(__dos_errlist[0]);

/* Values for error class (CLASS field): */
const char *__dos_errclass [] = {
/* 01h (1)  */ "Out of resource (storage space or I/O channels)",
/* 02h (2)  */ "Temporary situation (file or record lock)",
/* 03h (3)  */ "Authorization / permission problem (denied access)",
/* 04h (4)  */ "Internal system error (system software bug)",
/* 05h (5)  */ "Hardware failure",
/* 06h (6)  */ "System failure (configuration file missing or incorrect)",
/* 07h (7)  */ "Application program error",
/* 08h (8)  */ "Not found",
/* 09h (9)  */ "Bad format",
/* 0Ah (10) */ "Locked",
/* 0Bh (11) */ "Media error",
/* 0Ch (12) */ "Already exists / collision with existing item",
/* 0Dh (13) */ "Unknown / other",
/* 0Eh (14) */ "(Undoc) Cannot",
/* 0Fh (15) */ "(Undoc) Time"
/* (End of list) */ };

int __dos_ncls = sizeof(__dos_errclass) / sizeof(__dos_errclass[0]);

/* Values for suggested action (ACTION field): */
const char *__dos_erraction [] = {
/* 01h (01) */ "Retry",
/* 02h (02) */ "Delayed retry",
/* 03h (03) */ "Prompt user to reenter input",
/* 04h (04) */ "Abort after cleanup",
/* 05h (05) */ "Immediate abort",
/* 06h (06) */ "Ignore",
/* 07h (07) */ "Retry after user intervention"
/* (End of list) */ };

int __dos_nact = sizeof(__dos_erraction) / sizeof(__dos_erraction[0]);

/* Values for error locus (LOCUS field): */
const char *__dos_errlocus [] = {
/*   01h (01) */  "Unknown or not appropriate",
/*   02h (02) */  "Block device (disk error)",
/*   03h (03) */  "Network related",
/*   04h (04) */  "Serial device (timeout)/(PTS-DOS 6.51+ & S/DOS 1.0+) Character device",
/*   05h (05) */  "Memory related"
/* (End of list) */ };

int __dos_nloc = sizeof(__dos_errlocus) / sizeof(__dos_errlocus[0]);

static char *
_err_unknown(int errnum)
{
  static char ebuf[40];         /* 64-bit number + slop */
  char *cp;
  int v=1000000, lz=0;

  strcpy(ebuf, "Unknown error: ");
  cp = ebuf + strlen(ebuf);
  if (errnum < 0)
  {
    *cp++ = '-';
    errnum = -errnum;
  }
  while (v)
  {
    int d = errnum / v;
    if (d || lz || (v == 1))
    {
      *cp++ = d+'0';
      lz = 1;
    }
    errnum %= v;
    v /= 10;
  }

  return ebuf;
}

int
_dostrerr(struct _DOSERROR *p_error, struct _DOSERROR_STR *p_str)
{
  if (!p_error || !p_str)
  {
    errno = EINVAL;
    return -1;
  }

  if (p_error->exterror >= 0 && p_error->exterror < __dos_nerr)
    p_str->exterror_str = unconst(__dos_errlist[p_error->exterror], char *);
  else
    p_str->exterror_str = _err_unknown(p_error->exterror);

  if (strcmp(p_str->exterror_str, RESERVED) == 0)
    p_str->exterror_str = _err_unknown(p_error->exterror);

  if (p_error->class >= 0 && p_error->class < __dos_ncls)
  #ifdef __cplusplus
    p_str->errclass_str = unconst(__dos_errclass[(unsigned char)p_error->errclass], char *);
  #else
    p_str->class_str = unconst(__dos_errclass[(unsigned char)p_error->class], char *);
  #endif
  else
    p_str->class_str = _err_unknown(p_error->class);

  if (p_error->action >= 0 && p_error->action < __dos_nact)
    p_str->action_str = unconst(__dos_erraction[(unsigned char)p_error->action], char *);
  else
    p_str->action_str = _err_unknown(p_error->action);

  if (p_error->locus >= 0 && p_error->locus < __dos_nloc)
    p_str->locus_str = unconst(__dos_errlocus[(unsigned char)p_error->locus], char *);
  else
    p_str->locus_str = _err_unknown(p_error->locus);

  return 0;
}
