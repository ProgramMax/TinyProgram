// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef FINDKERNEL32_HPP
#define FINDKERNEL32_HPP


#include "PrecompiledHeader.hpp"

#include "Expected.hpp"


// Upon process launch, two important things happen:
// - Windows automatically links Kernel32.dll.
// - Windows sets a register pointing to the TEB. (FS register for 32-bit, GS register for 64-bit)

// From that TEB pointer, we can get the base address of the automatically-loaded Kernel32.dll.
// We can then use Kernel32.dll to access to all the other Windows functionality.

// Although the FS/GS registers are unlikely to ever be overwritten, we should still consider it a possibility
// and load Kernel32 early in app launch.


namespace TinyProgram {


namespace find_kernel32_error {
	enum Enum {
		Kernel32NotFound,
	};
}
Expected<HMODULE, find_kernel32_error::Enum> find_kernel32();


} // namespace TinyProgram


#endif // #ifndef FINDKERNEL32_HPP