/// @ref ext_matrix_int2x3_sized
/// @file linearmath/ext/matrix_int2x3_sized.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_int2x3_sized LINEARMATH_EXT_matrix_int2x3_sized
/// @ingroup ext
///
/// Include <linearmath/ext/matrix_int2x3_sized.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat2x3.hpp"
#include "../ext/scalar_int_sized.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_matrix_int2x3_sized extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_matrix_int2x3_sized
	/// @{

	/// 8 bit signed integer 2x3 matrix.
	///
	/// @see ext_matrix_int2x3_sized
	typedef mat<2, 3, int8, defaultp>				i8mat2x3;

	/// 16 bit signed integer 2x3 matrix.
	///
	/// @see ext_matrix_int2x3_sized
	typedef mat<2, 3, int16, defaultp>				i16mat2x3;

	/// 32 bit signed integer 2x3 matrix.
	///
	/// @see ext_matrix_int2x3_sized
	typedef mat<2, 3, int32, defaultp>				i32mat2x3;

	/// 64 bit signed integer 2x3 matrix.
	///
	/// @see ext_matrix_int2x3_sized
	typedef mat<2, 3, int64, defaultp>				i64mat2x3;

	/// @}
}//namespace linearmath
