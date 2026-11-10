/// @ref ext_vector_int2_sized
/// @file rblxmath/ext/vector_int2_sized.hpp
///
/// @defgroup ext_vector_int2_sized RBLXMATH_EXT_vector_int2_sized
/// @ingroup ext
///
/// Exposes sized signed integer vector of 2 components type.
///
/// Include <rblxmath/ext/vector_int2_sized.hpp> to use the features of this extension.
///
/// @see ext_scalar_int_sized
/// @see ext_vector_uint2_sized

#pragma once

#include "../ext/vector_int2.hpp"
#include "../ext/scalar_int_sized.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_vector_int2_sized extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_vector_int2_sized
	/// @{

	/// 8 bit signed integer vector of 2 components type.
	///
	/// @see ext_vector_int2_sized
	typedef vec<2, int8, defaultp>		i8vec2;

	/// 16 bit signed integer vector of 2 components type.
	///
	/// @see ext_vector_int2_sized
	typedef vec<2, int16, defaultp>		i16vec2;

	/// 32 bit signed integer vector of 2 components type.
	///
	/// @see ext_vector_int2_sized
	typedef vec<2, int32, defaultp>		i32vec2;

	/// 64 bit signed integer vector of 2 components type.
	///
	/// @see ext_vector_int2_sized
	typedef vec<2, int64, defaultp>		i64vec2;

	/// @}
}//namespace rblxmath
