// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef FINDKERNEL32VIASTACKWALK_HPP
#define FINDKERNEL32VIASTACKWALK_HPP


#include "Expected.hpp"


namespace TinyProgram {


namespace find_kernel32_via_stack_walk_error {
	enum Enum {
		Kernel32NotFound,
	};
}
Expected<char*, find_kernel32_via_stack_walk_error::Enum> find_kernel32_via_stalk_walk();


} // namespace TinyProgram


#endif // #ifndef FINDKERNEL32VIASTACKWALK_HPP