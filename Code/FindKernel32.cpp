// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "PrecompiledHeader.hpp"

#include "FindKernel32.hpp"

#include "FindKernel32ViaLDR.hpp"
//#include "FindKernel32ViaSEH.hpp"
#include "FindKernel32ViaStackWalk.hpp"


namespace {

	namespace find_kernel32_from_kernel32_code_error {
		enum Enum {
			Kernel32NotFound,
		};
	}
	TinyProgram::Expected<HMODULE, find_kernel32_from_kernel32_code_error::Enum> find_kernel32_from_kernel32_code(char* kernel32_code) {
		// Multiple methods of finding Kernel32 start by simply finding some code that lives inside Kernel32.
		// All of Kernel32's code will be loaded into memory as a contiguous part of the image.
		// We can trim off the low bits to get that page's starting address. Then we can just walk backwards
		// page by page until we find the "MZ" from the beginning of the DOS header.

		// Align the pointer to 4KiB pages.
		char* stack_address = reinterpret_cast<char*>(reinterpret_cast<unsigned long>(kernel32_code) & 0xFFFFF000);
		while(true) {
			if (stack_address[0] == 'M' && stack_address[1] == 'Z') {
				// found
				return reinterpret_cast<HMODULE>(stack_address);
			}
			stack_address -= 0x1000;
		}
	}


} // anonymous namespace


namespace TinyProgram {


Expected<HMODULE, find_kernel32_error::Enum> find_kernel32() {
	// Attempt to find Kernel32 via LDR first.
	// This works well on WinNT-based Windows but doesn't work on Win9x-based Windows.
	// So if the LDR method fails for this known reason, try a Win9x-friendly fallback.
	Expected<HMODULE, find_kernel32_via_ldr_error::Enum> kernel32_via_ldr = find_kernel32_via_ldr();
	if (!kernel32_via_ldr.has_value()) {
		// If the reason was the LDR could not be used, that likely means we're on a Win9x machine instead of a WinNT machine.
		// Try a Win9x-safe method instead:
		Expected<char*, find_kernel32_via_stack_walk_error::Enum> kernel32_via_stack_walk = find_kernel32_via_stalk_walk();
		if (!kernel32_via_stack_walk.has_value()) {
			return unexpected(find_kernel32_error::Kernel32NotFound);
		}

		Expected<HMODULE, find_kernel32_from_kernel32_code_error::Enum> kernel32 = find_kernel32_from_kernel32_code(kernel32_via_stack_walk.value());
		if (!kernel32.has_value()) {
			return unexpected(find_kernel32_error::Kernel32NotFound);
		}

		return kernel32.value();
	}

	return kernel32_via_ldr.value();
}


} // namespace TinyProgram