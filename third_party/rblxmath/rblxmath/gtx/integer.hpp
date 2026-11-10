/// @ref gtx_integer
/// @file rblxmath/gtx/integer.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_integer RBLXMATH_GTX_integer
/// @ingroup gtx
///
/// Include <rblxmath/gtx/integer.hpp> to use the features of this extension.
///
/// Add support for integer for core functions

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtc/integer.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_integer is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_integer extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_integer
	/// @{

	//! Returns x raised to the y power.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL int pow(int x, uint y);

	//! Returns the positive square root of x.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL int sqrt(int x);

	//! Returns the floor log2 of x.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL unsigned int floor_log2(unsigned int x);

	//! Modulus. Returns x - y * floor(x / y) for each component in x using the floating point value y.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL int mod(int x, int y);

	//! Return the factorial value of a number (!12 max, integer only)
	//! From RBLXMATH_GTX_integer extension.
	template<typename genType>
	RBLXMATH_FUNC_DECL genType factorial(genType const& x);

	//! 32bit signed integer.
	//! From RBLXMATH_GTX_integer extension.
	typedef signed int					sint;

	//! Returns x raised to the y power.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL uint pow(uint x, uint y);

	//! Returns the positive square root of x.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL uint sqrt(uint x);

	//! Modulus. Returns x - y * floor(x / y) for each component in x using the floating point value y.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL uint mod(uint x, uint y);

	//! Returns the number of leading zeros.
	//! From RBLXMATH_GTX_integer extension.
	RBLXMATH_FUNC_DECL uint nlz(uint x);

	/// @}
}//namespace rblxmath

#include "integer.inl"
