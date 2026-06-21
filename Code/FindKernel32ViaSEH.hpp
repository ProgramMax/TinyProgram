// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef FINDKERNEL32VIASEH_HPP
#define FINDKERNEL32VIASEH_HPP


#include "Expected.hpp"


// If two conditions are both true
//   - the target is 32-bit, and
//   - the CRT is linked
// then a Kernel32 entry will be added to the SEH handler list.

// Using the SEH handler list to find Kernel32 uses only 31 bytes of code (with /O1, minimize size).
// However, if the intention is to remove the CRT dependency, this method does not meet the required conditions.
// I've included this code to show it was thoroughly investigated and allow others to confirm & use for their own purposes. Perhaps they meet the two conditions.

// For our usage, we might as well use our own stack walking method.
// The CRT finds Kernel32 by walking the stack. We would be doing the same but skipping the SEH step.


namespace TinyProgram {


namespace find_kernel32_via_seh_error {
	enum Enum {
		Kernel32NotFound,
	};
}
Expected<char*, find_kernel32_via_seh_error::Enum> find_kernel32_via_seh();


} // namespace TinyProgram


#endif // #ifndef FINDKERNEL32VIASEH_HPP