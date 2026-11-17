/// @ref ext_vector_float1
/// @file linearmath/ext/vector_float1.hpp
///
/// @defgroup ext_vector_float1 LINEARMATH_EXT_vector_float1
/// @ingroup ext
///
/// Exposes single-precision floating point vector type with one component.
///
/// Include <linearmath/ext/vector_float1.hpp> to use the features of this extension.
///
/// @see ext_vector_float1_precision extension.
/// @see ext_vector_double1 extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_vector_float1 extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_vector_float1
	/// @{

	/// 1 components vector of single-precision floating-point numbers.
	typedef vec<1, float, defaultp>		vec1;

	/// @}
}//namespace linearmath
