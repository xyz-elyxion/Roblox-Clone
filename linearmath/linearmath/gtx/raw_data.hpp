/// @ref gtx_raw_data
/// @file linearmath/gtx/raw_data.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_raw_data LINEARMATH_GTX_raw_data
/// @ingroup gtx
///
/// Include <linearmath/gtx/raw_data.hpp> to use the features of this extension.
///
/// Projection of a vector to other one

#pragma once

// Dependencies
#include "../ext/scalar_uint_sized.hpp"
#include "../detail/setup.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_raw_data is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_raw_data extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_raw_data
	/// @{

	//! Type for byte numbers.
	//! From LINEARMATH_GTX_raw_data extension.
	typedef detail::uint8		byte;

	//! Type for word numbers.
	//! From LINEARMATH_GTX_raw_data extension.
	typedef detail::uint16		word;

	//! Type for dword numbers.
	//! From LINEARMATH_GTX_raw_data extension.
	typedef detail::uint32		dword;

	//! Type for qword numbers.
	//! From LINEARMATH_GTX_raw_data extension.
	typedef detail::uint64		qword;

	/// @}
}// namespace linearmath

#include "raw_data.inl"
