/// @ref gtx_hash
/// @file rblxmath/gtx/hash.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_hash RBLXMATH_GTX_hash
/// @ingroup gtx
///
/// Include <rblxmath/gtx/hash.hpp> to use the features of this extension.
///
/// Add std::hash support for rblxmath types

#pragma once

#if defined(RBLXMATH_FORCE_MESSAGES) && !defined(RBLXMATH_EXT_INCLUDED)
#	ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#		pragma message("RBLXMATH: RBLXMATH_GTX_hash is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it.")
#	else
#		pragma message("RBLXMATH: RBLXMATH_GTX_hash extension included")
#	endif
#endif

#include "../vec2.hpp"
#include "../vec3.hpp"
#include "../vec4.hpp"
#include "../gtc/vec1.hpp"

#include "../gtc/quaternion.hpp"
#include "../gtx/dual_quaternion.hpp"

#include "../mat2x2.hpp"
#include "../mat2x3.hpp"
#include "../mat2x4.hpp"

#include "../mat3x2.hpp"
#include "../mat3x3.hpp"
#include "../mat3x4.hpp"

#include "../mat4x2.hpp"
#include "../mat4x3.hpp"
#include "../mat4x4.hpp"

#if __cplusplus < 201103L
#pragma message("RBLXMATH_GTX_hash requires C++11 standard library support")
#endif

#if RBLXMATH_LANG & RBLXMATH_LANG_CXX11
#define RBLXMATH_GTX_hash 1
#include <functional>

namespace std
{
	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::vec<1, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::vec<1, T, Q> const& v) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::vec<2, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::vec<2, T, Q> const& v) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::vec<3, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::vec<3, T, Q> const& v) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::vec<4, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::vec<4, T, Q> const& v) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::qua<T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::qua<T, Q> const& q) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::tdualquat<T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::tdualquat<T,Q> const& q) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<2, 2, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<2, 2, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<2, 3, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<2, 3, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<2, 4, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<2, 4, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<3, 2, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<3, 2, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<3, 3, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<3, 3, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<3, 4, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<3, 4, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<4, 2, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<4, 2, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<4, 3, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<4, 3, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};

	template<typename T, rblxmath::qualifier Q>
	struct hash<rblxmath::mat<4, 4, T, Q> >
	{
		RBLXMATH_FUNC_DECL size_t operator()(rblxmath::mat<4, 4, T,Q> const& m) const RBLXMATH_NOEXCEPT;
	};
} // namespace std

#include "hash.inl"

#endif //RBLXMATH_LANG & RBLXMATH_LANG_CXX11
