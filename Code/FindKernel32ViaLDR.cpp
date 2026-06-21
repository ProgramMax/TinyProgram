// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "PrecompiledHeader.hpp"

#include "FindKernel32ViaLDR.hpp"


namespace {

	typedef struct _UNICODE_STRING {
		USHORT Length;
		USHORT MaximumLength;
		PWSTR  Buffer;
	} UNICODE_STRING;

	/*
	typedef struct _RTL_USER_PROCESS_PARAMETERS {
		BYTE Reserved1[16];
		PVOID Reserved2[10];
		UNICODE_STRING ImagePathName;
		UNICODE_STRING CommandLine;
	} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

	typedef VOID (NTAPI *PPS_POST_PROCESS_INIT_ROUTINE) (VOID);
	*/

	struct LDR {
		BYTE Reserved1[8];
		PVOID Reserved2[3];
		LIST_ENTRY InMemoryOrderModuleList;
	};

	struct LDR_ENTRY {
		PVOID Reserved1[2];
		LIST_ENTRY InMemoryOrderLinks;
		PVOID Reserved2[2];
		PVOID DllBase;
		PVOID Reserved3[2];
		UNICODE_STRING FullDllName;
		//unsigned char pad[116];
		//ULONG BaseNameHashValue;
	};

	struct ProcessExecutionBlock {
		BOOLEAN InheritedAddressSpace;
		BOOLEAN ReadImageFileExecOptions;
		BOOLEAN BeingDebugged;
		BOOLEAN SpareBool;
		HANDLE Mutant;
		HMODULE ImageBaseAddress;
		LDR* Ldr;
		//RTL_USER_PROCESS_PARAMETERS* ProcessParameters; // Using VOIDP below to skip definitions
		PVOID ProcessParameters;
		PVOID SubSystemData;
		HANDLE ProcessHeap;
		PRTL_CRITICAL_SECTION FastPebLock;
		PVOID /* PPEBLOCKROUTINE */ FastPebLockRoutine;
		PVOID /* PPEBLOCKROUTINE */ FastPebUnlockRoutine;
		ULONG EnvironmentUpdateCount;
		PVOID KernelCallbackTable;
		ULONG Reserved[2];
		PVOID /* PPEB_FREE_BLOCK */ FreeList;
		ULONG TlsExpansionCounter;
		PVOID /* PRTL_BITMAP */ TlsBitmap;
		ULONG TlsBitmapBits[2];
		PVOID ReadOnlySharedMemoryBase;
		PVOID ReadOnlySharedMemoryHeap;
		PVOID* ReadOnlyStaticServerData;
		PVOID AnsiCodePageData;
		PVOID OemCodePageData;
		PVOID UnicodeCaseTableData;
		ULONG NumberOfProcessors;
		ULONG NtGlobalFlag;
		LARGE_INTEGER CriticalSectionTimeout;
		SIZE_T HeapSegmentReserve;
		SIZE_T HeapSegmentCommit;
		SIZE_T HeapDeCommitTotalFreeThreshold;
		SIZE_T HeapDeCommitFreeBlockThreshold;
		ULONG NumberOfHeaps;
		ULONG MaximumNumberOfHeaps;
		PVOID* ProcessHeaps;
		PVOID GdiSharedHandleTable;
		PVOID ProcessStarterHelper;
		PVOID GdiDCAttributeList;
		PVOID LoaderLock;
		ULONG OSMajorVersion;
		ULONG OSMinorVersion;
		ULONG OSBuildNumber;
		ULONG OSPlatformId;
		ULONG ImageSubsystem;
		ULONG ImageSubsystemMajorVersion;
		ULONG ImageSubsystemMinorVersion;
	};


	bool is_ldr_entry_kernel32(LDR_ENTRY* ldr_entry) {
		// The DLL path is the full path.
		// Windows might be installed on a non-default drive/path so we cannot assume the complete.
		// Trim the path and get just the final file name.
		size_t last_path_separator_index = -1;
		for (size_t i = 0; i < ldr_entry->FullDllName.Length / sizeof(WCHAR); i++) {
			WCHAR character = ldr_entry->FullDllName.Buffer[i];
			if (character == '\\') {
				last_path_separator_index = i;
			}
		}


		// Compare to "kernel32.dll"
		// TODO: For this part, we *could* possibly compare against a known hash value. Investigate.
		size_t file_name_length = (ldr_entry->FullDllName.Length / sizeof(WCHAR)) - last_path_separator_index - 1;
		const size_t kernel32_length = 12; // 12 == len("kernel32.dll")
		if (file_name_length != kernel32_length) {
			return false;
		}
		const WCHAR target_name[] = L"kernel32.dll";
		for (size_t i2 = 0; i2 < file_name_length; i2++) {
			if (target_name[i2] != ldr_entry->FullDllName.Buffer[(ldr_entry->FullDllName.Length / sizeof(WCHAR)) - file_name_length + i2]) {
				return false;
			}
		}

		return true;
	}


} // anonymous namespace


namespace TinyProgram {


Expected<HMODULE, find_kernel32_via_ldr_error::Enum> find_kernel32_via_ldr() {
	// On WinNT-based Windows, the TEB contains a pointer to the PEB.
	// On Win9x-based Windows, that PEB pointer will be NULL and we cannot use the PEB approach.
	#if ! defined _MSC_VER
		#error "Assumes MSVC defines"
	#else
		#if defined( _M_IX86 )
			// On 32-bit processes, the 0x30 offset of FS points to the PEB.
			#if (_MSC_FULL_VER >= 13012035)
				const unsigned long PEB_offset = 0x30;
				unsigned long PEB = __readfsdword(PEB_offset);
				//PEB = __readfsdword (FIELD_OFFSET (TEB, ProcessEnvironmentBlock));
			#else
				unsigned long PEB;
				__asm {
					mov eax, fs:[0x30]
					mov PEB, eax
				}
			#endif
		#elif defined( _M_X64 ) // _M_AMD64
			// On 64-bit processes, the 0x60 offset of GS points to the PEB.
			const unsigned long PEB_offset = 0x60;
			unsigned __int64 PEB = __readgsqword(PEB_offset);
		#endif
	#endif

	ProcessExecutionBlock* peb = reinterpret_cast<ProcessExecutionBlock*>(PEB);
	// The PEB contains a pointer to the first entry in a doubly linked list.
	LDR* ldr = peb->Ldr;
	if (ldr == NULL) {
		// LDR only exists on WinNT-based Windows.
		// For Win9x-based Windows, Ldr is NULL and we cannot use it to find Kernel32.
		// Instead, 
		return unexpected(find_kernel32_via_ldr_error::LDRUnavailable);
	}


	// The first "node" just points into the list. It doesn't exist in the list itself.
	LIST_ENTRY list = ldr->InMemoryOrderModuleList;
	LIST_ENTRY* list_item = list.Flink;
    for ( ; ; list_item = list_item->Flink) {
		// The linked list node structure does not start at the beginning of the LDR structure.
		// Given the pointer to the linked list node structure, calculate the negative offset to get
		// to the beginning of the LDR structure.

		// The CONTAINING_RECORD macro is provided by Windows.h and calculates that negative offset.
        LDR_ENTRY* ldr_entry = CONTAINING_RECORD(list_item, LDR_ENTRY, InMemoryOrderLinks);
		if (is_ldr_entry_kernel32(ldr_entry)) {
			return reinterpret_cast<HMODULE>(ldr_entry->DllBase);
		}
    }

	return unexpected(find_kernel32_via_ldr_error::Kernel32NotFound);
}


} // namespace TinyProgram