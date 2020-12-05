;--------------------------------------------------------------------------
;  crtxinit.asm :- C run-time: copy XINIT to XISEG
;
;  Copyright (C) 2004, Erik Petrich
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License 
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;  might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

; Set DUAL_DPTR to 1 and reassemble if your derivative has dual data pointers
; Especially useful if movx @Ri cannot go beyond the first 256 bytes of xdata
; due to lack of P2 or _XPAGE
; If the derivative has auto-toggle or auto-increment it can be further optimized
	DUAL_DPTR = 0

	.area CSEG    (CODE)
	.area GSINIT0 (CODE)
	.area GSINIT1 (CODE)
	.area GSINIT2 (CODE)
	.area GSINIT3 (CODE)
	.area GSINIT4 (CODE)
	.area GSINIT5 (CODE)
	.area GSINIT  (CODE)
	.area GSFINAL (CODE)

	.area GSINIT3 (CODE)

	.if DUAL_DPTR

	.globl _DPS			; assume DPSEL is in DPS bit0

__mcs51_genXINIT::
	mov	r1,#l_XINIT
	mov	a,r1
	orl	a,#(l_XINIT >> 8)
	jz	00003$
	mov	r2,#((l_XINIT+255) >> 8)
	orl	_DPS,#0x01		; set DPSEL, select DPTR1
	mov	dptr,#s_XINIT		; DPTR1 for code
	dec	_DPS			; clear DPSEL, select DPTR0
	mov	dptr,#s_XISEG		; DPTR0 for xdata
00001$:	clr	a
	inc	_DPS			; set DPSEL, select DPTR1
	movc	a,@a+dptr
	inc	dptr
	dec	_DPS			; clear DPSEL, select DPTR0
	movx	@dptr,a
	inc	dptr
	djnz	r1,00001$
	djnz	r2,00001$
00003$:

	.else

	.globl __XPAGE

__mcs51_genXINIT::
	mov	r1,#l_XINIT
	mov	a,r1
	orl	a,#(l_XINIT >> 8)
	jz	00003$
	mov	r2,#((l_XINIT+255) >> 8)
	mov	dptr,#s_XINIT
	mov	r0,#s_XISEG
	mov	__XPAGE,#(s_XISEG >> 8)
00001$:	clr	a
	movc	a,@a+dptr
	movx	@r0,a
	inc	dptr
	inc	r0
	cjne	r0,#0,00002$
	inc	__XPAGE
00002$:	djnz	r1,00001$
	djnz	r2,00001$
	mov	__XPAGE,#0xFF
00003$:

	.endif
