// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DynamicLibrary.hpp"



namespace TinyProgram {



// GetProcAddress() from Kernel32.dll fails if using ordinals.
// This is almost certainly deliberate.

// The story goes a book, Unauthorized Windows 95, pointed out some secret functions
// which were exposed by Kernel32.dll. To stop people using them, the names were hidden
// from the exports list. Users could still access exported non-secret functions by name.

// However, this would still allow getting the secret functions by ordinal.
// So GetProcAddress() fails if the module is Kernel32 and the parameter is an ordinal.

// Kernel32.dll is still loaded into the process's address space, however. So those
// functions are still there. They're just a bit harder to find.

// The "solution" (read: workaround for this crappy obfuscation) is to parse the
// PE header of Kernel32.dll from within your address space
// https://ftp.st.ryukoku.ac.jp/pub/published/oreilly/windows/win95.update/dirty.html

namespace GetKernel32ProcAddressError {
	enum Enum {
		Kernel32NotLoaded,
		MagicNumberNotFound,
		RequestedOrdinalOutsideRange,
	};
}
static Expected<DWORD, GetKernel32ProcAddressError::Enum> GetKernel32ProcAddress(short ordinal) {
	HANDLE module = GetModuleHandle("Kernel32"); // This does not increment the refcount.
	if (module == NULL) {
		// GetLastError()
		return unexpected(GetKernel32ProcAddressError::Kernel32NotLoaded);
	}

	char* buffer = static_cast<char*>(module);

	// First is the DOS header. It starts with a special number.
	const int dos_header_magic_number = 0x5A4D;
	WORD* dos_header = reinterpret_cast<WORD*>(buffer);
	if (*dos_header != dos_header_magic_number) {
		return unexpected(GetKernel32ProcAddressError::MagicNumberNotFound);
	}

	// The end of the DOS header has an offset to the NT header.
	// This is useful because right after the DOS header is typically a small
	// DOS program that prints "This program cannot be run in DOS mode".
	// Microsoft's tooling also inserts something called the "Rich Header" after this program.
	// The Rich Header contains information about the tools and their versions used
	// to build the program.
	const int nt_header_offset_index = 0x3C;
	WORD nt_header_offset = *reinterpret_cast<WORD*>(&buffer[nt_header_offset_index]);
	IMAGE_NT_HEADERS* nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(&buffer[nt_header_offset]);

	const int nt_header_magic_number = 0x4550;
	if (nt_header->Signature != nt_header_magic_number) {
		return unexpected(GetKernel32ProcAddressError::MagicNumberNotFound);
	}

	DWORD export_directory_offset = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	IMAGE_EXPORT_DIRECTORY* export_directory = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(buffer + export_directory_offset);

	ordinal--; // Ordinals are 1-based. The table is 0-based.
	if (ordinal >= export_directory->NumberOfFunctions) {
		return unexpected(GetKernel32ProcAddressError::RequestedOrdinalOutsideRange);
	}

	DWORD* function_address_offsets = reinterpret_cast<DWORD*>(buffer + export_directory->AddressOfFunctions);
	

	return reinterpret_cast<DWORD>(buffer + function_address_offsets[ordinal]);
}



typedef DWORD (_stdcall *GetModuleFileNameA_type)(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
static GetModuleFileNameA_type get_module_file_name_g;

typedef BOOL (_stdcall *FreeLibrary_type)(HMODULE hLibModule);
static FreeLibrary_type free_library_g;

typedef HMODULE (_stdcall *LoadLibraryA_type)(LPCSTR lpLibFileName);
static LoadLibraryA_type load_library_g;

typedef FARPROC (_stdcall *GetProcAddress_type)(HMODULE hModule, LPCSTR lpProcName);
static GetProcAddress_type get_proc_address_g;


void setup_dynamic_library_system() {
	const short GetModuleFileNameA_ordinal = 395;
	Expected<DWORD, GetKernel32ProcAddressError::Enum> get_module_file_name = GetKernel32ProcAddress(GetModuleFileNameA_ordinal);
	if (!get_module_file_name.has_value()) {
		// TODO: handle error
	}
	get_module_file_name_g = reinterpret_cast<GetModuleFileNameA_type>(get_module_file_name.value());

	const short FreeLibrary_ordinal = 307;
	Expected<DWORD, GetKernel32ProcAddressError::Enum> free_library = GetKernel32ProcAddress(FreeLibrary_ordinal);
	if (!free_library.has_value()) {
		// TODO: handle error
	}
	free_library_g = reinterpret_cast<FreeLibrary_type>(free_library.value());

	const short LoadLibraryA_ordinal = 553;
	Expected<DWORD, GetKernel32ProcAddressError::Enum> load_library = GetKernel32ProcAddress(LoadLibraryA_ordinal);
	if (!load_library.has_value()) {
		// TODO: handle error
	}
	load_library_g = reinterpret_cast<LoadLibraryA_type>(load_library.value());

	const short GetProcAddress_ordinal = 419;
	Expected<DWORD, GetKernel32ProcAddressError::Enum> get_proc_address = GetKernel32ProcAddress(GetProcAddress_ordinal);
	if (!get_proc_address.has_value()) {
		// TODO: handle error
	}
	get_proc_address_g = reinterpret_cast<GetProcAddress_type>(get_proc_address.value());
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

	TCHAR file_name[MAX_PATH];
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
	FARPROC function = get_proc_address_g(module_, name);
	if (function == NULL) {
		// GetLastError();
		return NULL;
	}

	return function;
}



Expected<DynamicLibrary, open_dynamic_library_error::Enum> open_dynamic_library(const char* file_name) {
	HMODULE module = load_library_g(file_name);
	if (module == NULL) {
		return unexpected(open_dynamic_library_error::LibraryNotFound);
	}

	return DynamicLibrary(module);
}



} // namespace TinyProgram