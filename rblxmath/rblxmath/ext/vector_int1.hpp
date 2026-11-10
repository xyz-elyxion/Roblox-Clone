/// @ref ext_vector_int1
/// @file rblxmath/ext/vector_int1.hpp
///
/// @defgroup ext_vector_int1 RBLXMATH_EXT_vector_int1
/// @ingroup ext
///
/// Exposes ivec1 vector type.
///
/// Include <rblxmath/ext/vector_int1.hpp> to use the features of this extension.
///
/// @see ext_vector_uint1 extension.
/// @see ext_vector_int1_precision extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_vector_int1 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_vector_int1
	/// @{

	/// 1 component vector of signed integer numbers.
	typedef vec<1, int, defaultp>			ivec1;

	/// @}
}//namespace rblxmath

