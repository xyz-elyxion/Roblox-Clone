/// @ref ext_vector_int3_sized
/// @file linearmath/ext/vector_int3_sized.hpp
///
/// @defgroup ext_vector_int3_sized LINEARMATH_EXT_vector_int3_sized
/// @ingroup ext
///
/// Exposes sized signed integer vector of 3 components type.
///
/// Include <linearmath/ext/vector_int3_sized.hpp> to use the features of this extension.
///
/// @see ext_scalar_int_sized
/// @see ext_vector_uint3_sized

#pragma once

#include "../ext/vector_int3.hpp"
#include "../ext/scalar_int_sized.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_vector_int3_sized extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_vector_int3_sized
	/// @{

	/// 8 bit signed integer vector of 3 components type.
	///
	/// @see ext_vector_int3_sized
	typedef vec<3, int8, defaultp>		i8vec3;

	/// 16 bit signed integer vector of 3 components type.
	///
	/// @see ext_vector_int3_sized
	typedef vec<3, int16, defaultp>		i16vec3;

	/// 32 bit signed integer vector of 3 components type.
	///
	/// @see ext_vector_int3_sized
	typedef vec<3, int32, defaultp>		i32vec3;

	/// 64 bit signed integer vector of 3 components type.
	///
	/// @see ext_vector_int3_sized
	typedef vec<3, int64, defaultp>		i64vec3;

	/// @}
}//namespace linearmath
