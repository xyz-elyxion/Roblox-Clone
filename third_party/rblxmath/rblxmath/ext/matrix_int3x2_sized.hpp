/// @ref ext_matrix_int3x2_sized
/// @file rblxmath/ext/matrix_int3x2_sized.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_int3x2_sized RBLXMATH_EXT_matrix_int3x2_sized
/// @ingroup ext
///
/// Include <rblxmath/ext/matrix_int3x2_sized.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat3x2.hpp"
#include "../ext/scalar_int_sized.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_int3x2_sized extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_int3x2_sized
	/// @{

	/// 8 bit signed integer 3x2 matrix.
	///
	/// @see ext_matrix_int3x2_sized
	typedef mat<3, 2, int8, defaultp>				i8mat3x2;

	/// 16 bit signed integer 3x2 matrix.
	///
	/// @see ext_matrix_int3x2_sized
	typedef mat<3, 2, int16, defaultp>				i16mat3x2;

	/// 32 bit signed integer 3x2 matrix.
	///
	/// @see ext_matrix_int3x2_sized
	typedef mat<3, 2, int32, defaultp>				i32mat3x2;

	/// 64 bit signed integer 3x2 matrix.
	///
	/// @see ext_matrix_int3x2_sized
	typedef mat<3, 2, int64, defaultp>				i64mat3x2;

	/// @}
}//namespace rblxmath
