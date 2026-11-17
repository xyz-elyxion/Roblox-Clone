/// @ref gtc_reciprocal
/// @file linearmath/gtc/reciprocal.hpp
///
/// @see core (dependence)
///
/// @defgroup gtc_reciprocal LINEARMATH_GTC_reciprocal
/// @ingroup gtc
///
/// Include <linearmath/gtc/reciprocal.hpp> to use the features of this extension.
///
/// Define secant, cosecant and cotangent functions.

#pragma once

// Dependencies
#include "../detail/setup.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTC_reciprocal extension included")
#endif

#include "../ext/scalar_reciprocal.hpp"
#include "../ext/vector_reciprocal.hpp"

