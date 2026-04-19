// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DynamicLibrary.hpp"



namespace TinyProgram {

typedef DWORD (_stdcall *GetModuleFileNameA_type)(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
static GetModuleFileNameA_type get_module_file_name_g;

typedef BOOL (_stdcall *FreeLibrary_type)(HMODULE hLibModule);
static FreeLibrary_type free_library_g;

typedef HMODULE (_stdcall *LoadLibraryA_type)(LPCSTR lpLibFileName);
static LoadLibraryA_type load_library_g;


void setup_dynamic_library_system() {
	HMODULE kernel32 = GetModuleHandleA("Kernel32"); // This does not increment the refcount.
	if (kernel32 == NULL) {
		// GetLastError()
		//return unexpected(GetKernel32ProcAddressError::Kernel32NotLoaded);
		return;
	}

	get_module_file_name_g = (GetModuleFileNameA_type)GetProcAddress(kernel32, "GetModuleFileNameA");
	free_library_g = (FreeLibrary_type)GetProcAddress(kernel32, "FreeLibrary");
	load_library_g = (LoadLibraryA_type)GetProcAddress(kernel32, "LoadLibraryA");
}

static HMODULE duplicate_pseudo_handle_refcount(HMODULE module) {
	// In 16-bit Windows, HMODULES were real handles.
	// In 32-bit Windows, they became pseudo handles.
	// This means we cannot DuplicateHandle() it to increment its refcount.

	// (You can even VirtualQuery() a function pointer and cast its
	// MEMORY_BASIC_INFORMATION.BaseAddress back to the HMODULE.)

	// So we need a way of incrementing the refcount.
	// This is done by calling LoadLibrary() again.	
	// However, we have the HMODULE, not the file name which LoadLibrary() requires.

	// So the steps are:
	//   1. get the file name of the module
	//   2. LoadLibrary() it

	char file_name[MAX_PATH];
	DWORD bytes_filled_in_buffer = get_module_file_name_g(module, file_name, MAX_PATH);
	if (bytes_filled_in_buffer == 0) {
		return reinterpret_cast<HMODULE>(NULL);
		/*
		DWORD error_code = GetLastError();
		if (error_code == ERROR_INSUFFICIENT_BUFFER) {
		}
		// Note: In XP, this returns ERROR_SUCCESS instead??
		*/
	}

	HMODULE duplicated_module_handle = load_library_g(file_name);
	if (duplicated_module_handle == NULL) {
		// To handle this error, we would return NULL.
		// And that's what we're doing by returning the value, which was set to NULL.
		// So no extra error handling is necessary.
	}

	return duplicated_module_handle;
}

DynamicLibrary::DynamicLibrary(HMODULE module)
	: module_(module)
{}

DynamicLibrary::DynamicLibrary(const DynamicLibrary& rhs)
	: module_(duplicate_pseudo_handle_refcount(rhs.module_))
{}

DynamicLibrary::~DynamicLibrary() {
	if (free_library_g(module_) == 0) {
		// GetLastError();
	}
}

FARPROC DynamicLibrary::get_function(const char* name) {
	FARPROC function = GetProcAddress(module_, name);
	if (function == NULL) {
		// GetLastError();
		return NULL;
	}

	return function;
}



Expected<DynamicLibrary, open_dynamic_library_error::Enum> open_dynamic_library(const char* file_name) {
	HMODULE module = load_library_g(file_name);
	if (module == NULL) {
		// GetLastError();
		return unexpected(open_dynamic_library_error::LibraryNotFound);
	}

	return DynamicLibrary(module);
}



} // namespace TinyProgram