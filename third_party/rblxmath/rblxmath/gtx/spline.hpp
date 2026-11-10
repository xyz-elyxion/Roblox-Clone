/// @ref gtx_spline
/// @file rblxmath/gtx/spline.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_spline RBLXMATH_GTX_spline
/// @ingroup gtx
///
/// Include <rblxmath/gtx/spline.hpp> to use the features of this extension.
///
/// Spline functions

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtx/optimum_pow.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_spline is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_spline extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_spline
	/// @{

	/// Return a point from a catmull rom curve.
	/// @see gtx_spline extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType catmullRom(
		genType const& v1,
		genType const& v2,
		genType const& v3,
		genType const& v4,
		typename genType::value_type const& s);

	/// Return a point from a hermite curve.
	/// @see gtx_spline extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType hermite(
		genType const& v1,
		genType const& t1,
		genType const& v2,
		genType const& t2,
		typename genType::value_type const& s);

	/// Return a point from a cubic curve.
	/// @see gtx_spline extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType cubic(
		genType const& v1,
		genType const& v2,
		genType const& v3,
		genType const& v4,
		typename genType::value_type const& s);

	/// @}
}//namespace rblxmath

#include "spline.inl"
