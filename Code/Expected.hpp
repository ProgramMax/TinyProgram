// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TINYPROGRAM_EXPECTED_HPP
#define TINYPROGRAM_EXPECTED_HPP



#include "Badge.hpp"
#include "Variant.hpp"



namespace TinyProgram {



// Forward declarations
namespace impl {
	template< typename ErrorType >
	class UnexpectedType;
}

template< typename ErrorType >
impl::UnexpectedType< ErrorType > unexpected(const ErrorType& error);

template< typename ExpectedType, typename ErrorType >
class Expected;



// Pass keys
template< typename ErrorType >
class ConstructorPassKey {
	template< typename >
	friend impl::UnexpectedType< ErrorType > unexpected(const ErrorType& error);
	ConstructorPassKey() {}
};




namespace impl {

	template< typename ErrorType >
	class UnexpectedType {
	public:

		explicit UnexpectedType(const ErrorType& error, ConstructorPassKey< ErrorType >)
			: error_(error)
		{}

		template< typename ExpectedType >
		ErrorType get(Badge< Expected< ExpectedType, ErrorType > >) const { return error_; }

	private:
		ErrorType error_;
	};

}



template< typename ErrorType >
impl::UnexpectedType< ErrorType > unexpected(const ErrorType& error) {
	return impl::UnexpectedType< ErrorType >(error, ConstructorPassKey< ErrorType >());
}



template< typename ExpectedType, typename ErrorType >
class Expected {
public:
	Expected(const ExpectedType& expected)
		: variant_(expected)
	{}

	Expected(const impl::UnexpectedType< ErrorType >& unexpected)
		: variant_(unexpected.get(Badge< Expected< ExpectedType, ErrorType > >()))
	{}


	ExpectedType* operator ->() {
		return &variant_.Get1();
	}
	ExpectedType* operator *() {
		return &variant_.Get1();
	}
	bool has_value() {
		return variant_.index() == 0;
	}
	ExpectedType value() {
		return variant_.Get1();
	}

private:
	Variant< ExpectedType, ErrorType>  variant_;
};



} // namespace TinyProgram

#endif // #ifndef TINYPROGRAM_EXPECTED_HPP