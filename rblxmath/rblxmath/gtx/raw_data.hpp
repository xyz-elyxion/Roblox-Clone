/// @ref gtx_raw_data
/// @file rblxmath/gtx/raw_data.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_raw_data RBLXMATH_GTX_raw_data
/// @ingroup gtx
///
/// Include <rblxmath/gtx/raw_data.hpp> to use the features of this extension.
///
/// Projection of a vector to other one

#pragma once

// Dependencies
#include "../ext/scalar_uint_sized.hpp"
#include "../detail/setup.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_raw_data is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_raw_data extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_raw_data
	/// @{

	//! Type for byte numbers.
	//! From RBLXMATH_GTX_raw_data extension.
	typedef detail::uint8		byte;

	//! Type for word numbers.
	//! From RBLXMATH_GTX_raw_data extension.
	typedef detail::uint16		word;

	//! Type for dword numbers.
	//! From RBLXMATH_GTX_raw_data extension.
	typedef detail::uint32		dword;

	//! Type for qword numbers.
	//! From RBLXMATH_GTX_raw_data extension.
	typedef detail::uint64		qword;

	/// @}
}// namespace rblxmath

#include "raw_data.inl"
