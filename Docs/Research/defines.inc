; Copyright 2026, the TinyProgram contributors. All rights reserved.
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.


%ifndef TINYPROGRAM_DEFINES_INC
%define TINYPROGRAM_DEFINES_INC


; PE header

; --architecture
TINYPROGRAM_I386   equ 0x14c
TINYPROGRAM_X86_64 equ 0x8664

; --characteristics
TINYPROGRAM_RELOCATION_INFO_STRIPPED equ 1
TINYPROGRAM_EXECUTABLE               equ 2
TINYPROGRAM_OVER_2GB_AWARE           equ 0x20
TINYPROGRAM_BITS32                   equ 0x100
TINYPROGRAM_DEBUG_INFO_STRIPPED      equ 0x200
TINYPROGRAM_IS_DLL                   equ 0x2000

; --subsystem
TINYPROGRAM_GUI     equ 2
TINYPROGRAM_CONSOLE equ 3


; Optional header

TINYPROGRAM_PE32     equ 0x10b
TINYPROGRAM_PE32PLUS equ 0x20b


; Section table
TINYPROGRAM_CODE               equ 0x20
TINYPROGRAM_INITIALIZED_DATA   equ 0x40
TINYPROGRAM_UNINITIALIZED_DATA equ 0x80
TINYPROGRAM_SHARED             equ 0x10000000
TINYPROGRAM_EXECUTE            equ 0x20000000
TINYPROGRAM_READ               equ 0x40000000
TINYPROGRAM_WRITE              equ 0x80000000


%endif ; %ifndef TINYPROGRAM_DEFINES_INC