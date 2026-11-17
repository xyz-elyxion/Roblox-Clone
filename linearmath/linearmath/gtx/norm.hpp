/// @ref gtx_norm
/// @file linearmath/gtx/norm.hpp
///
/// @see core (dependence)
/// @see gtx_quaternion (dependence)
/// @see gtx_component_wise (dependence)
///
/// @defgroup gtx_norm LINEARMATH_GTX_norm
/// @ingroup gtx
///
/// Include <linearmath/gtx/norm.hpp> to use the features of this extension.
///
/// Various ways to compute vector norms.

#pragma once

// Dependency:
#include "../geometric.hpp"
#include "../gtx/quaternion.hpp"
#include "../gtx/component_wise.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_norm is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_norm extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_norm
	/// @{

	/// Returns the squared length of x.
	/// From LINEARMATH_GTX_norm extension.
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T length2(vec<L, T, Q> const& x);

	/// Returns the squared distance between p0 and p1, i.e., length2(p0 - p1).
	/// From LINEARMATH_GTX_norm extension.
	template<length_t L, typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T distance2(vec<L, T, Q> const& p0, vec<L, T, Q> const& p1);

	//! Returns the L1 norm between x and y.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T l1Norm(vec<3, T, Q> const& x, vec<3, T, Q> const& y);

	//! Returns the L1 norm of v.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T l1Norm(vec<3, T, Q> const& v);

	//! Returns the L2 norm between x and y.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T l2Norm(vec<3, T, Q> const& x, vec<3, T, Q> const& y);

	//! Returns the L2 norm of v.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T l2Norm(vec<3, T, Q> const& x);

	//! Returns the L norm between x and y.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T lxNorm(vec<3, T, Q> const& x, vec<3, T, Q> const& y, unsigned int Depth);

	//! Returns the L norm of v.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T lxNorm(vec<3, T, Q> const& x, unsigned int Depth);

	//! Returns the LMax norm between x and y.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T lMaxNorm(vec<3, T, Q> const& x, vec<3, T, Q> const& y);

	//! Returns the LMax norm of v.
	//! From LINEARMATH_GTX_norm extension.
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL T lMaxNorm(vec<3, T, Q> const& x);

	/// @}
}//namespace linearmath

#include "norm.inl"
