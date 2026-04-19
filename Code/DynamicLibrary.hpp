// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TINYPROGRAM_DYNAMICLIBRARY_HPP
#define TINYPROGRAM_DYNAMICLIBRARY_HPP



#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>


#include "Expected.hpp"



namespace TinyProgram {



void setup_dynamic_library_system();



class DynamicLibrary {
public:
	explicit DynamicLibrary(HMODULE module);
	DynamicLibrary(const DynamicLibrary& rhs);
	~DynamicLibrary();

	template< typename FunctionType >
	FunctionType get_function(const char* name, FunctionType /*deduction clue*/) {
		return reinterpret_cast<FunctionType>(get_function(name));
	}

	template< typename FunctionType >
	FunctionType get_function(short ordinal, FunctionType /*deduction clue*/) {
		return reinterpret_cast<FunctionType>(get_function(reinterpret_cast<const char *>(ordinal)));
	}

private:
	FARPROC get_function(const char* name);

	HMODULE module_;
};



namespace open_dynamic_library_error {
	enum Enum {
		LibraryNotFound,
	};
}
Expected<DynamicLibrary, open_dynamic_library_error::Enum> open_dynamic_library(const char* file_name);



} // namespace TinyProgram

#endif // #ifndef TINYPROGRAM_DYNAMICLIBRARY_HPP