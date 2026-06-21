// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef PRECOMPILEDHEADER_HPP
#define PRECOMPILEDHEADER_HPP


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>



#if (_MSC_FULL_VER >= 13012035)
	#include <intrin.h>

	#if defined( _M_IX86 )
		#pragma intrinsic(__readfsdword)
	#elif defined( _M_X64 )
		#pragma intrinsic(__readgsqword)
	#endif

	#pragma intrinsic(_AddressOfReturnAddress)
#endif

#endif // #ifndef PRECOMPILEDHEADER_HPP