// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TINYPROGRAM_VARIANT_HPP
#define TINYPROGRAM_VARIANT_HPP



#include <new>



namespace TinyProgram {
namespace impl {

	template< typename T1, typename T2 >
	struct MaxSize {
		enum size { value = sizeof(T1) > sizeof(T2) ? sizeof(T1) : sizeof(T2) };
	};

} // namespace impl



template<typename T1, typename T2>
class Variant {
public:

	explicit Variant(const T1& rhs)
		: which_(0)
	{
		T1* t1_data = reinterpret_cast<T1*>(&data_);
		new (t1_data) T1(rhs);
	}

	explicit Variant(const T2& rhs)
		: which_(1)
	{
		T2* t2_data = reinterpret_cast<T2*>(&data_);
		new (t2_data) T2(rhs);
	}


	T1& Get1() { return *reinterpret_cast<T1*>(&data_); }
	T2& Get2() { return *reinterpret_cast<T2*>(&data_); }

	char index() { return which_; }

private:
	// char aliases with everything. So we use char as the backing data type.
	char data_[impl::MaxSize<T1, T2>::size::value];
	char which_;
};



} // namespace TinyProgram

#endif // #ifndef TINYPROGRAM_VARIANT_HPP