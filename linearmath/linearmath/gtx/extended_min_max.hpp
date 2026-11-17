/// @ref gtx_extended_min_max
/// @file linearmath/gtx/extended_min_max.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_extended_min_max LINEARMATH_GTX_extended_min_max
/// @ingroup gtx
///
/// Include <linearmath/gtx/extended_min_max.hpp> to use the features of this extension.
///
/// Min and max functions for 3 to 4 parameters.

#pragma once

// Dependency:
#include "../linearmath.hpp"
#include "../ext/vector_common.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_extended_min_max is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_extended_min_max extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_extended_min_max
	/// @{

	/// Return the minimum component-wise values of 3 inputs
	/// @see gtx_extented_min_max
	template<typename T>
	LINEARMATH_FUNC_DECL T min(
		T const& x,
		T const& y,
		T const& z);

	/// Return the minimum component-wise values of 3 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> min(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z);

	/// Return the minimum component-wise values of 3 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> min(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z);

	/// Return the minimum component-wise values of 4 inputs
	/// @see gtx_extented_min_max
	template<typename T>
	LINEARMATH_FUNC_DECL T min(
		T const& x,
		T const& y,
		T const& z,
		T const& w);

	/// Return the minimum component-wise values of 4 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> min(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z,
		typename C<T>::T const& w);

	/// Return the minimum component-wise values of 4 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> min(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z,
		C<T> const& w);

	/// Return the maximum component-wise values of 3 inputs
	/// @see gtx_extented_min_max
	template<typename T>
	LINEARMATH_FUNC_DECL T max(
		T const& x,
		T const& y,
		T const& z);

	/// Return the maximum component-wise values of 3 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> max(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z);

	/// Return the maximum component-wise values of 3 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> max(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z);

	/// Return the maximum component-wise values of 4 inputs
	/// @see gtx_extented_min_max
	template<typename T>
	LINEARMATH_FUNC_DECL T max(
		T const& x,
		T const& y,
		T const& z,
		T const& w);

	/// Return the maximum component-wise values of 4 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> max(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z,
		typename C<T>::T const& w);

	/// Return the maximum component-wise values of 4 inputs
	/// @see gtx_extented_min_max
	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_DECL C<T> max(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z,
		C<T> const& w);

	/// @}
}//namespace linearmath

#include "extended_min_max.inl"
