// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef TINYPROGRAM_MEMORY_HPP
#define TINYPROGRAM_MEMORY_HPP


#include "PrecompiledHeader.hpp"

#include <new>


typedef LPVOID (_stdcall *VirtualAlloc_type)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
static VirtualAlloc_type virtual_alloc_g;
//MEM_COMMIT 0x00001000
//PAGE_READWRITE 0x04

typedef BOOL (_stdcall *VirtualFree_type)(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
static VirtualFree_type virtual_free_g;
//MEM_RELEASE 0x00008000



void setup_memory_system();



inline void* __cdecl operator new(size_t size) {
	LPVOID pointer = virtual_alloc_g(NULL, size, MEM_COMMIT, PAGE_READWRITE);
	//LPVOID pointer = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
	return pointer;
}

/*
inline void* __cdecl operator new(unsigned int size, void* pointer) {
	return pointer;
}
*/

inline void __cdecl operator delete(void* pointer) {
	if (pointer) {
		BOOL result = virtual_free_g(pointer, 0, MEM_RELEASE);
		//BOOL result = VirtualFree(pointer, 0, MEM_RELEASE);
	}
}


#endif // #ifndef TINYPROGRAM_MEMORY_HPP