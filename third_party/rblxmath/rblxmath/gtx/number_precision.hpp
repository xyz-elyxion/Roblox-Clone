/// @ref gtx_number_precision
/// @file rblxmath/gtx/number_precision.hpp
///
/// @see core (dependence)
/// @see gtc_type_precision (dependence)
/// @see gtc_quaternion (dependence)
///
/// @defgroup gtx_number_precision RBLXMATH_GTX_number_precision
/// @ingroup gtx
///
/// Include <rblxmath/gtx/number_precision.hpp> to use the features of this extension.
///
/// Defined size types.

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtc/type_precision.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_number_precision is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_number_precision extension included")
#endif

namespace rblxmath{
	/////////////////////////////
	// Unsigned int vector types

	/// @addtogroup gtx_number_precision
	/// @{

	//////////////////////
	// Float matrix types

	typedef f32			f32mat1;	//!< \brief Single-qualifier floating-point scalar. (from RBLXMATH_GTX_number_precision extension)
	typedef f32			f32mat1x1;	//!< \brief Single-qualifier floating-point scalar. (from RBLXMATH_GTX_number_precision extension)
	typedef f64			f64mat1;	//!< \brief Double-qualifier floating-point scalar. (from RBLXMATH_GTX_number_precision extension)
	typedef f64			f64mat1x1;	//!< \brief Double-qualifier floating-point scalar. (from RBLXMATH_GTX_number_precision extension)

	/// @}
}//namespace rblxmath

