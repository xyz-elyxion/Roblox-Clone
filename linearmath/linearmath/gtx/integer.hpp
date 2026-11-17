/// @ref gtx_integer
/// @file linearmath/gtx/integer.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_integer LINEARMATH_GTX_integer
/// @ingroup gtx
///
/// Include <linearmath/gtx/integer.hpp> to use the features of this extension.
///
/// Add support for integer for core functions

#pragma once

// Dependency:
#include "../linearmath.hpp"
#include "../gtc/integer.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_integer is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_integer extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_integer
	/// @{

	//! Returns x raised to the y power.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL int pow(int x, uint y);

	//! Returns the positive square root of x.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL int sqrt(int x);

	//! Returns the floor log2 of x.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL unsigned int floor_log2(unsigned int x);

	//! Modulus. Returns x - y * floor(x / y) for each component in x using the floating point value y.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL int mod(int x, int y);

	//! Return the factorial value of a number (!12 max, integer only)
	//! From LINEARMATH_GTX_integer extension.
	template<typename genType>
	LINEARMATH_FUNC_DECL genType factorial(genType const& x);

	//! 32bit signed integer.
	//! From LINEARMATH_GTX_integer extension.
	typedef signed int					sint;

	//! Returns x raised to the y power.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL uint pow(uint x, uint y);

	//! Returns the positive square root of x.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL uint sqrt(uint x);

	//! Modulus. Returns x - y * floor(x / y) for each component in x using the floating point value y.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL uint mod(uint x, uint y);

	//! Returns the number of leading zeros.
	//! From LINEARMATH_GTX_integer extension.
	LINEARMATH_FUNC_DECL uint nlz(uint x);

	/// @}
}//namespace linearmath

#include "integer.inl"
