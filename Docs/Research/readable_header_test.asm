; Copyright 2026, the TinyProgram contributors. All rights reserved.
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.


; This hard-codes the header structures so we can see and manipulate them.
;
; Any values which are unused are zeroed with "times <size> db 0".
; Values which are read by the Windows loader will be explicitly populated.
; The intention is to make it easier to manipulate.
; The exception to this rule is the data directories at the end of the optional header.
; Comments there explain its special case.


%include "defines.inc"


; These are the minimum values that the Win9x loaders allow
; TODO: It seems like 1 works for file_alignment on Win98. Check others.
file_alignment    equ 0x200
section_alignment equ 0x1000
; TODO: Apparently, some versions of Windows will allow section alignment smaller than a page size.
; It'll just round up to a page size.

%define round(n, r) (((n+(r-1))/r)*r)


dos_header:
	db 'MZ'
	times 58 db 0
	dd pe_header - dos_header
dos_header_end:


; DOS stub here
;bits 16


; Rich header here


; The PE docs say the PE header needs to be 8-byte aligned.
; This is not true on Win98 at least. But I need to test others.
; I think maybe XP 32-bit requires 4-byte alignment?
;times round($-$$, 8)-($-$$) db 0

pe_header:
	db 'PE', 0, 0
	dw TINYPROGRAM_I386
	dw 0 ; section count
	times 12 db 0
	dw optional_header_size
	dw TINYPROGRAM_BITS32 | TINYPROGRAM_EXECUTABLE | TINYPROGRAM_RELOCATION_INFO_STRIPPED | TINYPROGRAM_DEBUG_INFO_STRIPPED
pe_header_end:


optional_header:
	optional_magic equ TINYPROGRAM_PE32
	dw optional_magic
	times 14 db 0
	dd entry_point
	times 4 db 0
%if optional_magic == TINYPROGRAM_PE32
	; PE32+ removed this field
	times 4 db 0
%endif

	image_base equ 0x400000
%if optional_magic == TINYPROGRAM_PE32
	dd image_base
%elif optional_magic == TINYPROGRAM_PE32PLUS
	dq image_base
%endif

	dd section_alignment
	dd file_alignment
	times 8 db 0
	dw 4, 0 ; subsystem version (major, minor)
	times 4 db 0
	dd round(file_size, section_alignment) ; size of image
	dd round(header_size, file_alignment) ; size of headers
	times 4 db 0
	dw TINYPROGRAM_GUI
	times 2 db 0

	; TODO: Apparently stack reserve and heap commit will be set to some
	; decent default values if the values are zero.
%if optional_magic == TINYPROGRAM_PE32
	dd 0x100000 ; stack reserve
	dd 0x1000 ; stack commit
	dd 0x100000 ; heap reserve
	dd 0x1000 ; heap commit
%elif optional_magic == TINYPROGRAM_PE32PLUS
	dq 0x100000 ; stack reserve
	dq 0x1000 ; stack commit
	dq 0x100000 ; heap reserve
	dq 0x1000 ; heap commit
%endif

	times 4 db 0
	data_directory_count equ 5
	; Win95 needs >= 5
	; Win98 needs data in the binary for all 16 entries.
	; A bug in the Win9x loader that checks values beyond what it should.
	; Apparently, XP (32-bit?) also reads the debug directory, ignoring the count.
	dd data_directory_count
	times (data_directory_count*8) db 0
	; The data directories are (in order):
	; - exports
	; - imports
	; - resources
	; - exception
	; - security
	; - base relocation table
	; - debug
	; - architecture specific
	; - global pointer RVA
	; - thread local storage callbacks
	; - load config
	; - bound imports
	; - import address table
	; - delay load imports
	; - COM runtime descriptor
optional_header_end:
optional_header_size equ optional_header_end - optional_header

; Section table here
; Each section entry is comprised of:
; - name (8 bytes)
; - virtual size (4 bytes)
; - virtual address (4 bytes, RVA)
; - size of raw data (4 bytes, rounded up to file alignmnet)
; - pointer to raw data (4 bytes, offset from file beginning)
; - pointer to relocations (4 bytes)
; - pointer to line numbers (4 bytes)
; - number of relocations (2 bytes)
; - number of line numbers (2 bytes)
; - characteristics (4 bytes)

; Win2k apparently requires an import section which includes Kernel32

header_size equ $ - $$


; Even though we're only specifying perhaps 5 data directory entries,
; Win9x requires the binary contain data for all 16.
; So we pad out the extra entries.
times (16-data_directory_count)*8 db 0


code:
bits 32
entry_point:
	ret
code_size equ $ - code



file_size equ $-$$


%include "tests.inc"