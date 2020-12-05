;--------------------------------------------------------------------------
;  crtxpop.asm - C run-time: pop registers (not bits) from xstack
;
;  Copyright (C) 2009, Maarten Brock
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

;--------------------------------------------------------
; overlayable bit register bank
;--------------------------------------------------------
	.area BIT_BANK	(REL,OVR,DATA)
bits:
	.ds 1

	ar0 = 0x00
	ar1 = 0x01

	.area HOME    (CODE)

; Pop registers r1..r7 & bits from xstack
; Expect mask in B
___sdcc_xpop_regs::
	mov	a,r0
	mov	r0,_spx
___sdcc_xpop::
	push acc
	jbc	B.0,00100$	;if B(0)=0 then
	dec	r0
	movx	a,@r0		;pop bits
	mov	bits,a
00100$:
	jbc	B.1,00101$	;if B(1)=0 then
	dec	r0
	movx	a,@r0		;pop R1
	mov	r1,a
00101$:
	jbc	B.2,00102$	;if B(2)=0 then
	dec	r0
	movx	a,@r0		;pop R2
	mov	r2,a
00102$:
	jbc	B.3,00103$	;if B(3)=0 then
	dec	r0
	movx	a,@r0		;pop R3
	mov	r3,a
00103$:
	jbc	B.4,00104$	;if B(4)=0 then
	dec	r0
	movx	a,@r0		;pop R4
	mov	r4,a
00104$:
	jbc	B.5,00105$	;if B(5)=0 then
	dec	r0
	movx	a,@r0		;pop R5
	mov	r5,a
00105$:
	jbc	B.6,00106$	;if B(6)=0 then
	dec	r0
	movx	a,@r0		;pop R6
	mov	r6,a
00106$:
	jbc	B.7,00107$	;if B(7)=0 then
	dec	r0
	movx	a,@r0		;pop R7
	mov	r7,a
00107$:
	mov	_spx,r0
	pop	ar0
	ret
