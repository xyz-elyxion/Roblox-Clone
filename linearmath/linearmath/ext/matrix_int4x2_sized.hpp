/// @ref ext_matrix_int4x2_sized
/// @file linearmath/ext/matrix_int4x2_sized.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_int4x2_sized LINEARMATH_EXT_matrix_int4x2_sized
/// @ingroup ext
///
/// Include <linearmath/ext/matrix_int4x2_sized.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat4x2.hpp"
#include "../ext/scalar_int_sized.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_matrix_int4x2_sized extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_matrix_int4x2_sized
	/// @{

	/// 8 bit signed integer 4x2 matrix.
	///
	/// @see ext_matrix_int4x2_sized
	typedef mat<4, 2, int8, defaultp>				i8mat4x2;

	/// 16 bit signed integer 4x2 matrix.
	///
	/// @see ext_matrix_int4x2_sized
	typedef mat<4, 2, int16, defaultp>				i16mat4x2;

	/// 32 bit signed integer 4x2 matrix.
	///
	/// @see ext_matrix_int4x2_sized
	typedef mat<4, 2, int32, defaultp>				i32mat4x2;

	/// 64 bit signed integer 4x2 matrix.
	///
	/// @see ext_matrix_int4x2_sized
	typedef mat<4, 2, int64, defaultp>				i64mat4x2;

	/// @}
}//namespace linearmath
