// Copyright 2026, the TinyProgram contributors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef TINYPROGRAM_VARIANT_HPP
#define TINYPROGRAM_VARIANT_HPP


#include "Memory.hpp"


namespace TinyProgram {
namespace impl {

	template< int Val1, int Val2 >
	struct Max {
		enum size { value = Val1 > Val2 ? Val1 : Val2 };
	};

	template< int InstanceNumber >
	struct BlankType {
	};

	template< int SizeInBytes >
	class VariantStorage {
		char data_[SizeInBytes];
	};

} // namespace impl



template< typename T1, typename T2 = impl::BlankType<2>/*, typename T3 = impl::BlankType<3>, typename T4 = impl::BlankType<4>*/ >
class Variant {
public:

	explicit Variant(const T1& rhs)
		: which_(1)
	{
		T1* t1_data = reinterpret_cast<T1*>(&data_);
		new (t1_data) T1(rhs);
	}

	explicit Variant(const T2& rhs)
		: which_(2)
	{
		T2* t2_data = reinterpret_cast<T2*>(&data_);
		new (t2_data) T2(rhs);
	}

	/*
	explicit Variant(const T3& rhs)
		: which_(3)
	{
		T3* t3_data = reinterpret_cast<T3*>(&data_);
		new (t3_data) T3(rhs);
	}

	explicit Variant(const T4& rhs)
		: which_(4)
	{
		T4* t4_data = reinterpret_cast<T4*>(&data_);
		new (t4_data) T4(rhs);
	}
	*/

	~Variant() {
		switch (which_) {
		case 1:
			Get1().~T1();
			break;
		case 2:
			Get2().~T2();
			break;
		/*
		case 3:
			Get3().~T3();
			break;
		case 4:
			Get4().~T4();
			break;
		*/
		}
	}

	T1& Get1() { return *reinterpret_cast<T1*>(&data_); }
	T2& Get2() { return *reinterpret_cast<T2*>(&data_); }
	//T3& Get3() { return *reinterpret_cast<T3*>(&data_); }
	//T4& Get4() { return *reinterpret_cast<T4*>(&data_); }

	char index() const { return which_; }

private:
	// char aliases with everything. So we use char as the backing data type.
	char data_[
#if _MSC_VER >= 1200
	#pragma warning(push)
#endif
#pragma warning(disable: 4042) // Nested template parameters are causing "bad storage class"
		//impl::Max<
			//impl::Max<
				impl::Max<sizeof(T1), sizeof(T2)>::value//,
				//sizeof(T3)
			//>::value,
			//sizeof(T4)
		//>::value
#if _MSC_VER >= 1200
	#pragma warning(pop)
#else
	#pragma warning(default:4042)
#endif
		];
	char which_;
};


} // namespace TinyProgram


#endif // #ifndef TINYPROGRAM_VARIANT_HPP