/// @ref ext_matrix_uint2x2
/// @file linearmath/ext/matrix_uint2x2.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_uint2x2 LINEARMATH_EXT_matrix_uint2x2
/// @ingroup ext
///
/// Include <linearmath/ext/matrix_uint2x2.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat2x2.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_matrix_uint2x2 extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_matrix_uint2x2
	/// @{

	/// Unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2
	typedef mat<2, 2, uint, defaultp>	umat2x2;

	/// Unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2
	typedef mat<2, 2, uint, defaultp>	umat2;

	/// @}
}//namespace linearmath
