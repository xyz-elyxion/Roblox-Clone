/// @ref ext_vector_bool1
/// @file rblxmath/ext/vector_bool1.hpp
///
/// @defgroup ext_vector_bool1 RBLXMATH_EXT_vector_bool1
/// @ingroup ext
///
/// Exposes bvec1 vector type.
///
/// Include <rblxmath/ext/vector_bool1.hpp> to use the features of this extension.
///
/// @see ext_vector_bool1_precision extension.

#pragma once

#include "../detail/type_vec1.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_vector_bool1 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_vector_bool1
	/// @{

	/// 1 components vector of boolean.
	typedef vec<1, bool, defaultp>		bvec1;

	/// @}
}//namespace rblxmath
