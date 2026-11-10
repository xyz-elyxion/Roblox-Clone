#pragma once

#include "setup.hpp"

namespace rblxmath{
namespace detail
{
	typedef short hdata;

	RBLXMATH_FUNC_DECL float toFloat32(hdata value);
	RBLXMATH_FUNC_DECL hdata toFloat16(float const& value);

}//namespace detail
}//namespace rblxmath

#include "type_half.inl"
