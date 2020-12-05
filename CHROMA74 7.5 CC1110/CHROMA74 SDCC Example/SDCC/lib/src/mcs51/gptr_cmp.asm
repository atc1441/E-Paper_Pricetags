;--------------------------------------------------------------------------
;  gptr_cmp.asm - C run-time: compare two generic pointers
;
;  Copyright (C) 2011, Maarten Brock
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

	.area HOME    (CODE)
	.area GSINIT0 (CODE)
	.area GSINIT1 (CODE)
	.area GSINIT2 (CODE)
	.area GSINIT3 (CODE)
	.area GSINIT4 (CODE)
	.area GSINIT5 (CODE)
	.area GSINIT  (CODE)
	.area GSFINAL (CODE)
	.area CSEG    (CODE)

	.area HOME    (CODE)

; compares two generic pointers.
; if p1 < p2  return NZ and C
; if p1 == p2 return  Z and NC
; if p1 > p2  return NZ and NC
; if both are a NULL pointer (yes, we have several) also return Z and NC
; assumes that banks never map to address 0x0000
; so it suffices to check dptr part only and ignore b

___gptr_cmp::
	mov  a,sp
	add  a,#0xfc
	clr  c
	xch  a,r0
	push acc
	push dpl
	mov  a,@r0
	inc  r0
	orl  a,@r0
	jnz  00001$
	mov  a,dpl
	orl  a,dph
; if both are NULL, return Z and NC
	jz   00002$
00001$:
	dec  r0
	mov  a,dpl
	subb a,@r0
	mov  dpl,a
	inc  r0
	mov  a,dph
	subb a,@r0
	orl  dpl,a
	inc  r0
	mov  a,b
	subb a,@r0
	orl  a,dpl
; p2 < p1, return NZ and C
; p2 = p1, return Z and NC
; p2 > p1, return NZ and NC
00002$:
	pop  dpl
	xch  a,r0
	pop  acc
	xch  a,r0
	ret
