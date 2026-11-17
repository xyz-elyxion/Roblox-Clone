/// @ref ext_vector_int1
/// @file linearmath/ext/vector_int1.hpp
///
/// @defgroup ext_vector_int1 LINEARMATH_EXT_vector_int1
/// @ingroup ext
///
/// Exposes ivec1 vector type.
///
/// Include <linearmath/ext/vector_int1.hpp> to use the features of this extension.
///
/// @see ext_vector_uint1 extension.
/// @see ext_vector_int1_precision extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_vector_int1 extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_vector_int1
	/// @{

	/// 1 component vector of signed integer numbers.
	typedef vec<1, int, defaultp>			ivec1;

	/// @}
}//namespace linearmath

