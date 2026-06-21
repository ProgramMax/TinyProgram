// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "PrecompiledHeader.hpp"

#include "DynamicLibrary.hpp"

#include "FindKernel32.hpp"


namespace TinyProgram {

typedef DWORD (_stdcall *GetModuleFileNameA_type)(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
static GetModuleFileNameA_type get_module_file_name_g;

typedef BOOL (_stdcall *FreeLibrary_type)(HMODULE hLibModule);
static FreeLibrary_type free_library_g;

typedef HMODULE (_stdcall *LoadLibraryA_type)(LPCSTR lpLibFileName);
static LoadLibraryA_type load_library_g;

typedef DWORD (_stdcall *GetLastError_type)();
static GetLastError_type get_last_error_g;

typedef FARPROC (_stdcall *GetProcAddress_type)(HMODULE hModule, LPCSTR lpProcName);
static GetProcAddress_type get_proc_address_g;




#pragma optimize("q", on)

unsigned int djb2(unsigned char* str) {
    unsigned int hash = 5381;
    unsigned char c;

    while (c = *(str++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

void get_export_table_from_module() {
	// Programs will be loaded into virtual memory addres 0x400'000.
	const long module_location = 0x400000;
	//const long kernel32_location = 0x415000; // __IMPORT_DESCRIPTOR_KERNEL32

	char* buffer = reinterpret_cast<char*>(module_location);
	//char* buffer = reinterpret_cast<char*>(kernel32_location);

	const int dos_header_magic_number = 0x5a4d;
	WORD* dos_header = reinterpret_cast<WORD*>(buffer);
	if (*dos_header != dos_header_magic_number) {
		return;
	}

	const int nt_header_offset_index = 0x3c;
	WORD nt_header_offset = *reinterpret_cast<WORD*>(&buffer[nt_header_offset_index]);
	IMAGE_NT_HEADERS* nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(&buffer[nt_header_offset]);

	const int nt_header_magic_number = 0x4550;
	if (nt_header->Signature != nt_header_magic_number) {
		return;
	}


	DWORD import_directory_offset = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD size = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	IMAGE_IMPORT_DESCRIPTOR* import_directory = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(buffer + import_directory_offset);
}





static IMAGE_EXPORT_DIRECTORY* get_exports(HMODULE module) {
	char* buffer = reinterpret_cast<char*>(module);

	const int nt_header_offset_index = 0x3C;
	WORD nt_header_offset = *reinterpret_cast<WORD*>(&buffer[nt_header_offset_index]);
	IMAGE_NT_HEADERS* nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(&buffer[nt_header_offset]);

	const int nt_header_magic_number = 0x4550;
	if (nt_header->Signature != nt_header_magic_number) {
		//return unexpected(GetKernel32ProcAddressError::MagicNumberNotFound);
		return NULL;
	}

	DWORD export_directory_offset = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	return reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(buffer + export_directory_offset);
}

bool do_strings_match(char* string1, char* string2) {
	size_t i = 0;
	while (string1[i] == string2[i]) {
		if (string1[i] == '\0' && string2[i] == '\0') {
			return true;
		}
		// If one string ends before the other, the NULL character won't match the other string and we'll fall out of the loop.
		++i;
	}

	return false;
}

unsigned short get_ordinal(HMODULE module, IMAGE_EXPORT_DIRECTORY* export_directory, char* function_name) {
	char* buffer = reinterpret_cast<char*>(module);

	DWORD* name_table = reinterpret_cast<DWORD*>(buffer + export_directory->AddressOfNames);
	DWORD name_count = export_directory->NumberOfNames;
	for (DWORD i = 0; i < name_count; i++) {
		char* name = reinterpret_cast<char*>(buffer + name_table[i]);
		if (do_strings_match(name, function_name)) {
			// Found

			unsigned short* ordinal_table = reinterpret_cast<unsigned short*>(buffer + export_directory->AddressOfNameOrdinals);
			return ordinal_table[i];
		}
	}

	return 0;
}

void* get_function_pointer_from_ordinal(HMODULE module, IMAGE_EXPORT_DIRECTORY* export_directory, unsigned short ordinal) {
	char* buffer = reinterpret_cast<char*>(module);

	//ordinal--; // Ordinals are 1-based. The table is 0-based.
	DWORD function_count = export_directory->NumberOfFunctions;
	if (ordinal >= export_directory->NumberOfFunctions) {
		//return unexpected(GetKernel32ProcAddressError::RequestedOrdinalOutsideRange);
		return NULL;
	}

	DWORD* function_address_offsets = reinterpret_cast<DWORD*>(buffer + export_directory->AddressOfFunctions);
	return reinterpret_cast<void*>(buffer + function_address_offsets[ordinal]);
}

Expected<char, setup_dynamic_library_system_error::Enum> setup_dynamic_library_system() {
	Expected<HMODULE, find_kernel32_error::Enum> kernel32 = find_kernel32();
	if (!kernel32.has_value()) {
		return setup_dynamic_library_system_error::Kernel32NotFound;
	}

	IMAGE_EXPORT_DIRECTORY* export_directory = get_exports(kernel32.value());

	// TODO: This would be a great place to use known hashes instead of string literals.
	unsigned short ordinal = get_ordinal(kernel32.value(), export_directory, "GetModuleFileNameA");
	get_module_file_name_g = reinterpret_cast<GetModuleFileNameA_type>(get_function_pointer_from_ordinal(kernel32.value(), export_directory, ordinal));

	ordinal = get_ordinal(kernel32.value(), export_directory, "FreeLibrary");
	free_library_g = reinterpret_cast<FreeLibrary_type>(get_function_pointer_from_ordinal(kernel32.value(), export_directory, ordinal));

	ordinal = get_ordinal(kernel32.value(), export_directory, "LoadLibraryA");
	load_library_g = reinterpret_cast<LoadLibraryA_type>(get_function_pointer_from_ordinal(kernel32.value(), export_directory, ordinal));

	ordinal = get_ordinal(kernel32.value(), export_directory, "GetLastError");
	get_last_error_g = reinterpret_cast<GetLastError_type>(get_function_pointer_from_ordinal(kernel32.value(), export_directory, ordinal));

	ordinal = get_ordinal(kernel32.value(), export_directory, "GetProcAddress");
	get_proc_address_g = reinterpret_cast<GetProcAddress_type>(get_function_pointer_from_ordinal(kernel32.value(), export_directory, ordinal));

	return static_cast<char>(0);
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
		DWORD error_code = get_last_error_g();
		if (error_code == ERROR_INSUFFICIENT_BUFFER) {
		}
		// Note: In XP, this returns ERROR_SUCCESS instead??
		return reinterpret_cast<HMODULE>(NULL);
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
	//FreeLibrary(module_);
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
		// GetLastError();
		return unexpected(open_dynamic_library_error::LibraryNotFound);
	}

	return DynamicLibrary(module);
}

#pragma optimize("q", off)


} // namespace TinyProgram