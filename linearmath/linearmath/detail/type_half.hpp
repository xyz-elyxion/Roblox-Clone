#pragma once

#include "setup.hpp"

namespace linearmath{
namespace detail
{
	typedef short hdata;

	LINEARMATH_FUNC_DECL float toFloat32(hdata value);
	LINEARMATH_FUNC_DECL hdata toFloat16(float const& value);

}//namespace detail
}//namespace linearmath

#include "type_half.inl"
