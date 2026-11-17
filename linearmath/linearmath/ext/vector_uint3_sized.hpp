/// @ref ext_vector_uint3_sized
/// @file linearmath/ext/vector_uint3_sized.hpp
///
/// @defgroup ext_vector_uint3_sized LINEARMATH_EXT_vector_uint3_sized
/// @ingroup ext
///
/// Exposes sized unsigned integer vector of 3 components type.
///
/// Include <linearmath/ext/vector_uint3_sized.hpp> to use the features of this extension.
///
/// @see ext_scalar_uint_sized
/// @see ext_vector_int3_sized

#pragma once

#include "../ext/vector_uint3.hpp"
#include "../ext/scalar_uint_sized.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_vector_uint3_sized extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_vector_uint3_sized
	/// @{

	/// 8 bit unsigned integer vector of 3 components type.
	///
	/// @see ext_vector_uint3_sized
	typedef vec<3, uint8, defaultp>		u8vec3;

	/// 16 bit unsigned integer vector of 3 components type.
	///
	/// @see ext_vector_uint3_sized
	typedef vec<3, uint16, defaultp>	u16vec3;

	/// 32 bit unsigned integer vector of 3 components type.
	///
	/// @see ext_vector_uint3_sized
	typedef vec<3, uint32, defaultp>	u32vec3;

	/// 64 bit unsigned integer vector of 3 components type.
	///
	/// @see ext_vector_uint3_sized
	typedef vec<3, uint64, defaultp>	u64vec3;

	/// @}
}//namespace linearmath
