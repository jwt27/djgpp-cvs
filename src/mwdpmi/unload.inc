; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

_unload_server:
	call	_cleanup		; Fixes ds
	mov	es, [_prefixseg]
	mov	ah, DOS_DEALLOCATE_MEMORY
	mov	ss, [_unload_server_seg]
	mov	sp, 0x100
	.db	OP_JMP_FAR
	.dw	DOS_CMDLINE_OFFSET, 0
_unload_server_seg = . - 2
; ---------------------------------------------------------------------------
