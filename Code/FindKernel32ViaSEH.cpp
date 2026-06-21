// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "PrecompiledHeader.hpp"

#include "FindKernel32ViaSEH.hpp"


// Official PEB/TEB definitions can be found in winternl.h, on SDKs that include it.
// However, those definitions hide things we need.
// For example, https://learn.microsoft.com/en-us/windows/win32/api/winternl/ns-winternl-teb
// notice the struct starts with Reserved1 where there would be SEH frames.

// Using these secretive hidden members is highly discouraged.
// Generally, any values needed from the PEB/TEB have corresponding Win32 retrieval functions which are preferred.
// However,
// 1.) we cannot use Win32 functions since we are in a nothing-linked state, and
// 2.) these values are well documented (unofficially) and used by compiler-generated code. See:
// https://web.archive.org/web/20090614054226/http://www.microsoft.com/msj/archive/s2ce.aspx
// Since the compiler itself relies on these members, they must be preserved and respected by Windows.
// So it is safe for us to rely on them.
namespace {

	struct ExceptionRegistration {
		ExceptionRegistration* Next;
		char* Handler;
	};

	struct ThreadExecutionBlock {
		ExceptionRegistration* CurrentSEHFrame;
		LPVOID StackBase; // high address
		LPVOID StackLimit; // low address
		LPVOID SubSystemTIB; // NT only?
		LPVOID FiberData;
		LPVOID ArbitraryData;
		LPVOID TIB;
		LPVOID dummy;
	};

} // anonymous namespace


namespace TinyProgram {


Expected<char*, find_kernel32_via_seh_error::Enum> find_kernel32_via_seh() {
	char* kernel32_code = NULL;

	// 64-bit Windows uses a stack unwinding table the program provides to the kernel.
	// This means we will not be able to walk a SEH list (which lives in user land) looking for Kernel32.
	#if defined(_M_IX86)
		#if (_MSC_FULL_VER >= 13012035)
			ThreadExecutionBlock* teb = reinterpret_cast<ThreadExecutionBlock*>(__readfsdword(0));
		#else
			ThreadExecutionBlock* teb;
			__asm {
				mov eax, fs:[0]
				mov teb, eax
			}
		#endif


		// If we want to be more safe, we could check that teb != NULL before accessing teb->CurrentSEHFrame below.
		// Same with checking current_seh_frame != NULL prior to looping.
		// But we're relying on Windows filling in that value with something valid.
		// If it contains junk, checking agianst NULL isn't much protection--we have bigger problems
		// and will only detect the problem if it happens to be one specific value.
		// Given the check isn't terribly helpful, takes up bytes, and should never happens, I skip it.

		// The final SEH frame is automatically inserted by Kernel32.
		// When we find it, we'll find code that lives inside Kernel32's memory.
		ExceptionRegistration* current_seh_frame = teb->CurrentSEHFrame;
		while (current_seh_frame != (void*)0xFFFFFFFF) {
			kernel32_code = current_seh_frame->Handler;
			current_seh_frame = current_seh_frame->Next;
		}
	#endif // #if defined(_M_IX86)

	if (!kernel32_code) {
		return unexpected(find_kernel32_via_seh_error::Kernel32NotFound);
	}

	return kernel32_code;
}


} // namespace TinyProgram