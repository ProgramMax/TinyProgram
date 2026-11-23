; Copyright 2026, the TinyProgram contributors. All rights reserved.
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.


; To use these tests:
;
; %include "defines.inc"
;
; dos_header:
;   ...
; dos_header_end:
;
; pe_header:
;   ...
; pe_header_end:
;
; optional_header:
;   optional_magic equ PE32
;   dw optional_magic
;   ...
;   data_directory_count equ 5
;   times data_directory_count*8 db 0
; optional_header_end:
;
; %include "tests.inc"
;
;
; optional_magic can be set to either PE32 or PE32PLUS


%ifndef TINYPROGRAM_TESTS_INC
%define TINYPROGRAM_TESTS_INC


%push tests_scope


%assign dos_header_size (dos_header_end - dos_header)
%if dos_header_size != 64
	%error DOS header size incorrect - expected 64, computed dos_header_size
%endif


%assign pe_header_size (pe_header_end - pe_header)
%if pe_header_size != 24
	%error PE header size incorrect - expected 24, computed pe_header_size
%endif


%assign optional_header_size (optional_header_end - optional_header)
%if optional_magic == TINYPROGRAM_PE32
	%assign expected_optional_header_size (96 + (data_directory_count * 8))
	%if optional_header_size != expected_optional_header_size
		%error Optional header size incorrect - expected expected_optional_header_size, computed optional_header_size
	%endif
%elif optional_magic == TINYPROGRAM_PE32PLUS
	%assign expected_optional_header_size (112 + (data_directory_count * 8))
	%if optional_header_size != expected_optional_header_size
		%error Optional header size incorrect - expected expected_optional_header_size, computed optional_header_size
	%endif
%endif


%pop ; %push tests_scope


%endif ; %ifndef TINYPROGRAM_TESTS_INC