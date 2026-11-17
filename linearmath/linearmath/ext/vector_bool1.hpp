/// @ref ext_vector_bool1
/// @file linearmath/ext/vector_bool1.hpp
///
/// @defgroup ext_vector_bool1 LINEARMATH_EXT_vector_bool1
/// @ingroup ext
///
/// Exposes bvec1 vector type.
///
/// Include <linearmath/ext/vector_bool1.hpp> to use the features of this extension.
///
/// @see ext_vector_bool1_precision extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_vector_bool1 extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_vector_bool1
	/// @{

	/// 1 components vector of boolean.
	typedef vec<1, bool, defaultp>		bvec1;

	/// @}
}//namespace linearmath
