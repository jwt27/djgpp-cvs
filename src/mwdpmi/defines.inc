; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

;; Special formating instructions for this file:
;;  1. Lines starting with ";;" will be ignored completely.
;;  2. Comments may not contain "/*" and "*/" (subject to rule 1).
;;  3. Blank (white-space only) lines are OK.
;;  4. Lines of the form "VAR = expr" are OK provided the expression is
;;     acceptable for both assembler and C.  You might want to include
;;     parenthesis.

; ---------------------------------------------------------------------------

; DPMI related.

DPMI_VERSION_MAJOR	= 1
DPMI_VERSION_MINOR	= 00

DPMI_OEM_VERSION_MAJOR	= 0
DPMI_OEM_VERSION_MINOR	= 10

; Masks for 0x0400.
DPMI_MASK_16		= (0 << 0)
DPMI_MASK_32		= (1 << 0)
DPMI_MASK_REFL_V86	= (0 << 1)
DPMI_MASK_REFL_REAL	= (1 << 1)
DPMI_MASK_NO_VIRT_MEM	= (0 << 2)
DPMI_MASK_VIRT_MEM	= (1 << 2)

; Masks for 0x0401.
DPMI_MASK_NO_PAGE_DIRTY		= (0 << 0)
DPMI_MASK_PAGE_DIRTY		= (1 << 0)
DPMI_MASK_NO_EXCEPTION_RESTART	= (0 << 1)
DPMI_MASK_EXCEPTION_RESTART	= (1 << 1)
DPMI_MASK_NO_DEVICE_MAPPING	= (0 << 2)
DPMI_MASK_DEVICE_MAPPING	= (1 << 2)
DPMI_MASK_NO_1M_MAPPING		= (0 << 3)
DPMI_MASK_1M_MAPPING		= (1 << 3)
DPMI_MASK_NO_DEMAND_ZERO_FILL	= (0 << 4)
DPMI_MASK_DEMAND_ZERO_FILL	= (1 << 4)
DPMI_MASK_NO_WRITE_PROTECT_CLIENT=(0 << 5)
DPMI_MASK_WRITE_PROTECT_CLIENT	= (1 << 5)
DPMI_MASK_NO_WRITE_PROTECT_HOST	= (0 << 6)
DPMI_MASK_WRITE_PROTECT_HOST	= (1 << 6)


; Multiplex (0x2f) functions.
DPMI_YIELD		= 0x1680
DPMI_GET_CPU_MODE	= 0x1686
DPMI_INSTALL_CHECK	= 0x1687

INT_DPMI_MULT		= 0x2f

REAL_MODE_STACK_SIZE	= 0x200
CALLBACK_COUNT		= 32
CALLBACK_SIZE		= 4
; ---------------------------------------------------------------------------
; Processor related.

FLAG_CF			= 0		; Carry
FLAG_PF			= 2		; Parity even
FLAG_AF			= 4		; Auxiliary carry
FLAG_ZF			= 6		; Zero
FLAG_SF			= 7		; Sign negative
FLAG_TF			= 8		; Trap
FLAG_IF			= 9		; Interrupt enable
FLAG_DF			= 10		; Direction up
FLAG_OF			= 11		; Overflow
FLAG_IOPL		= 12		; I/O privilege level (2 bits)
FLAG_NT			= 14		; Nested task
FLAG_RF			= 16		; Resume
FLAG_VM			= 17		; V86 mode
FLAG_AC			= 18		; Alignment check
FLAG_ID			= 21		; CPUID
FLAGS_USER		= ((1 << FLAG_CF) | (1 << FLAG_PF) | (1 << FLAG_AF) | (1 << FLAG_ZF) | (1 << FLAG_SF) | (1 << FLAG_DF) | (1 << FLAG_OF))


FLAG_CR0_PE		= 0		; Protected mode
FLAG_CR0_MP		= 1		; Math present (always 1 for 486+)
FLAG_CR0_EM		= 2		; Emulate
FLAG_CR0_TS		= 3		; Task switched
FLAG_CR0_ET		= 4		; Extension type (386?)
FLAG_CR0_NE		= 5		; Numeric error
FLAG_CR0_WP		= 16		; Write protect (user against superv)
FLAG_CR0_AM		= 18		; Alignment mask
FLAG_CR0_NW		= 29		; Not write-through
FLAG_CR0_CD		= 30		; Cache disable
FLAG_CR0_PG		= 31		; Paging enabled

FLAG_CR3_PWT		= 3		; Page-Level writes transparent
FLAG_CR3_PCD		= 4		; Page-Level cache disable

OP_OPSIZE		= 0x66
OP_ADDRSIZE		= 0x67
OP_PUSHW		= 0x68
OP_PUSHD		= ((OP_PUSHW << 8) | OP_OPSIZE)
OP_PUSHB		= 0x6a
OP_CALLF		= 0x9a
OP_MOV_AX		= 0xb8
OP_NEAR_RET		= 0xc3
OP_INT			= 0xcd
OP_JMP_FAR		= 0xea
OP_JMP_SHORT		= 0xeb
OP_SLOWDOWN		= 0x00eb

; Definitions for page tables.
PT_P			= 0x001	; present (else not)
PT_W			= 0x002	; writable (else read-only)
PT_U			= 0x004	; user mode (else kernel mode)
PT_T			= 0x008	; page write-through (else write-back)
PT_CD			= 0x010	; page caching disabled (else enabled)
PT_A			= 0x020	; accessed (else not)
PT_D			= 0x040	; dirty (else clean)
; Available bits and their CWSDPMI use:
; PT_I			= 0x200	; Initialized (else contents undefined)
; PT_S			= 0x400	; Swappable (else not)
; PT_C			= 0x800	; Candidate for swapping

PAGE_SIZE_LOG		= 12
PAGE_SIZE		= (1 << PAGE_SIZE_LOG)
KB_SIZE_LOG		= 10
KB_SIZE			= (1 << KB_SIZE_LOG)
; ---------------------------------------------------------------------------
; Hardware I/O ports

PS2_PORT_A		= 0x92
PS2_PORT_A_A20		= 0x02
PORT_KBD_DATA		= 0x60
PORT_KBD_STAT		= 0x64

; ---------------------------------------------------------------------------
; GDT layout

gdt_zero_sel		= (0 << 3)
gdt_core_sel		= (1 << 3)
gdt_code16_sel		= (2 << 3)
gdt_data16_sel		= (3 << 3)
gdt_code32_sel		= (4 << 3)
gdt_data32_sel		= (5 << 3)
gdt_ldt_sel		= (6 << 3)
gdt_tss_sel		= (7 << 3)
gdt_bios_sel		= (8 << 3)	; 0x40 compat
gdt_goreal_sel		= (9 << 3)
gdt_savestate_sel	= (10 << 3)
gdt_vcpi0_sel		= (11 << 3)
gdt_vcpi1_sel		= (12 << 3)
gdt_vcpi2_sel		= (13 << 3)

ldt_selector_count	= 128

SERVER_LINEAR		= 0x400000	; 4MB, 2nd page table entry

; ---------------------------------------------------------------------------
; XMS services

INT_XMS			= 0x2f

XMS_CHECK		= 0x4300
XMS_GET_ENTRY		= 0x4310
XMS_LOCAL_ENABLE_A20	= 0x05
XMS_LOCAL_DISABLE_A20	= 0x06
XMS_ALLOCATE		= 0x09
XMS_FREE		= 0x0a
XMS_LOCK		= 0x0c
XMS_UNLOCK		= 0x0d

; The interrupt list says:
;   HIMEM.SYS requires at least 256 bytes free stack space when calling
;   the driver entry point

; ---------------------------------------------------------------------------
; DOS services

INT_DOS			= 0x21

DOS_READ_STDIN		= 0x07
DOS_PRINT_STRING	= 0x09
DOS_SET_INT_VEC		= 0x25
DOS_GET_VERSION		= 0x30
DOS_TSR			= 0x31
DOS_GET_INT_VEC		= 0x35
DOS_OPEN_FILE		= 0x3d
DOS_CLOSE_FILE		= 0x3e
DOS_WRITE_FILE		= 0x40
DOS_ALLOCATE_MEMORY	= 0x48
DOS_DEALLOCATE_MEMORY	= 0x49
DOS_RESIZE_MEMORY	= 0x4a
DOS_EXIT		= 0x4c
DOS_GET_ALLOCATION_STRATEGY	= 0x5800
DOS_SET_ALLOCATION_STRATEGY	= 0x5801
DOS_GET_UMB_STATUS	= 0x5802
DOS_SET_UMB_STATUS	= 0x5803
DOS_GET_PSP		= 0x62

DOS_STDOUT_FILENO	= 1
DOS_STDERR_FILENO	= 2

DOS_ENVIRONMENT_OFFSET	= 0x2c
DOS_CMDLINE_OFFSET	= 0x80

INT_VDISK		= 0x19
; ---------------------------------------------------------------------------
; EMS services

INT_EMS			= 0x67
EMS_ALLOCATE		= 0x43
EMS_DEALLOCATE		= 0x45

; ---------------------------------------------------------------------------
; VCPI services

INT_VCPI		= INT_EMS
VCPI_PRESENT		= 0xde00
VCPI_INTERFACE		= 0xde01
VCPI_MAX_PHYMEMADR	= 0xde02
VCPI_MEM_CAPACITY	= 0xde03
VCPI_ALLOC_PAGE		= 0xde04
VCPI_FREE_PAGE		= 0xde05
VCPI_GET_PHYMEMADR	= 0xde06
VCPI_READ_CR0		= 0xde07
VCPI_READ_DR		= 0xde08
VCPI_SET_DR		= 0xde09
VCPI_GET_PIC_VECTOR	= 0xde0a
VCPI_SET_PIC_VECTOR	= 0xde0b
VCPI_MODE_CHANGE	= 0xde0c
; ---------------------------------------------------------------------------
; Various memory sources.

MEMORY_FROM_VCPI	= 1
MEMORY_FROM_XMS		= 2
MEMORY_FROM_INT15	= 3
; ---------------------------------------------------------------------------
; BIOS services.

; Int 0x15
BIOS_EXT_MEM_SIZE	= 0x88
; ---------------------------------------------------------------------------
