// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "PrecompiledHeader.hpp"

#include "FindKernel32ViaStackWalk.hpp"


namespace {

	struct StackFrame {
		struct StackFrame* ebp;
		char* eip;
	};

} // anonymous namespace


namespace TinyProgram {


Expected<char*, find_kernel32_via_stack_walk_error::Enum> find_kernel32_via_stalk_walk() {
	// The call frames before ours will come from Kernel32 on Win9x machines or NTDLL on WinNT machines.
	// This function assumes it is only being called on Win9x machines.

	// Win9x loads all system DLLs above 2GiB. User code lives below that.
	// So we walk the stack until we see an instruction address above 0x80000000.
	// That instruction address is within Kernel32.

	#if (_MSC_FULL_VER >= 13012035)
		StackFrame* stack_frame = (StackFrame*)_AddressOfReturnAddress();
	#else
		StackFrame* stack_frame;
		__asm {
			mov stack_frame, ebp
		}
	#endif

	char* kernel32_code = NULL;
	while (stack_frame && reinterpret_cast<unsigned long>(stack_frame->eip) < 0x80000000) {
		kernel32_code = stack_frame->eip;
		stack_frame = stack_frame->ebp;
	}

	if (!kernel32_code) {
		return unexpected(find_kernel32_via_stack_walk_error::Kernel32NotFound);
	}

	return stack_frame->eip;
}


} // namespace TinyProgram