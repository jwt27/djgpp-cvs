/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define FUNC(x)		.globl x; x:

/* The following are not needed since you must define the structures to pass
   to them; and dpmi.h defines both the structure and the procedure ... */

FUNC(__go32_dpmi_get_free_memory_information)
	jmp ___dpmi_get_free_memory_information

FUNC(__go32_dpmi_simulate_int)
	jmp ___dpmi_simulate_real_mode_interrupt

FUNC(__go32_dpmi_simulate_fcall)
	jmp ___dpmi_simulate_real_mode_procedure_retf

FUNC(__go32_dpmi_simulate_fcall_iret)
	jmp ___dpmi_simulate_real_mode_procedure_iret

/* These, however, may have been used without prototypes.  Compatibility. */

FUNC(__go32_my_cs)
	jmp __my_cs

FUNC(__go32_my_ds)
	jmp __my_ds

FUNC(__go32_my_ss)
	jmp __my_ss

FUNC(__go32_conventional_mem_selector)
	movzwl __go32_info_block+26,%eax
	ret
