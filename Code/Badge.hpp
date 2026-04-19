// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TINYPROGRAM_BADGE_HPP
#define TINYPROGRAM_BADGE_HPP



// Badge<T> is used to grant fine-grained access control to a function.
// Think of how 'friend' grants access to all private members.
// Badge<T> grants access to just a specific member fucntion.

// It works like this:
// class Accessor;
// class Protected{
//	int foo(Badge<Accessor>){ return 0; }
// };
// class Accessor {
//	int access_protected(Protected& p) { return p.foo({}); }
// };

// Note: In older compilers / C++ standards, you cannot use the {} construction.
// You must instead "return p.foo(Badge<Accessor>());"

namespace TinyProgram {



template<typename T>
class Badge {
	friend T;
	Badge() {} // WARNING: This must be {} (not =default) to prevent aggregate initialization.
	// Enabling C++20 will also disable aggregate initialization.
};



} // namespace TinyProgram

#endif // #ifndef TINYPROGRAM_BADGE_HPP