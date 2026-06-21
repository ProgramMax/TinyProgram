// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef FINDKERNEL32VIALDR_HPP
#define FINDKERNEL32VIALDR_HPP


#include "PrecompiledHeader.hpp"

#include "Expected.hpp"


namespace TinyProgram {


namespace find_kernel32_via_ldr_error {
	enum Enum {
		LDRUnavailable,
		Kernel32NotFound,
	};
}
Expected<HMODULE, find_kernel32_via_ldr_error::Enum> find_kernel32_via_ldr();


} // namespace TinyProgram


#endif // #ifndef FINDKERNEL32VIALDR_HPP