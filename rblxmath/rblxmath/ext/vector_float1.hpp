/// @ref ext_vector_float1
/// @file rblxmath/ext/vector_float1.hpp
///
/// @defgroup ext_vector_float1 RBLXMATH_EXT_vector_float1
/// @ingroup ext
///
/// Exposes single-precision floating point vector type with one component.
///
/// Include <rblxmath/ext/vector_float1.hpp> to use the features of this extension.
///
/// @see ext_vector_float1_precision extension.
/// @see ext_vector_double1 extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_vector_float1 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_vector_float1
	/// @{

	/// 1 components vector of single-precision floating-point numbers.
	typedef vec<1, float, defaultp>		vec1;

	/// @}
}//namespace rblxmath
