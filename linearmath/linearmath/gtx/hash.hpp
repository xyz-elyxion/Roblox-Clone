/// @ref gtx_hash
/// @file linearmath/gtx/hash.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_hash LINEARMATH_GTX_hash
/// @ingroup gtx
///
/// Include <linearmath/gtx/hash.hpp> to use the features of this extension.
///
/// Add std::hash support for linearmath types

#pragma once

#if defined(LINEARMATH_FORCE_MESSAGES) && !defined(LINEARMATH_EXT_INCLUDED)
#	ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#		pragma message("LINEARMATH: LINEARMATH_GTX_hash is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it.")
#	else
#		pragma message("LINEARMATH: LINEARMATH_GTX_hash extension included")
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
#pragma message("LINEARMATH_GTX_hash requires C++11 standard library support")
#endif

#if LINEARMATH_LANG & LINEARMATH_LANG_CXX11
#define LINEARMATH_GTX_hash 1
#include <functional>

namespace std
{
	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::vec<1, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::vec<1, T, Q> const& v) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::vec<2, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::vec<2, T, Q> const& v) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::vec<3, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::vec<3, T, Q> const& v) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::vec<4, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::vec<4, T, Q> const& v) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::qua<T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::qua<T, Q> const& q) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::tdualquat<T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::tdualquat<T,Q> const& q) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<2, 2, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<2, 2, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<2, 3, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<2, 3, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<2, 4, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<2, 4, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<3, 2, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<3, 2, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<3, 3, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<3, 3, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<3, 4, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<3, 4, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<4, 2, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<4, 2, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<4, 3, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<4, 3, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};

	template<typename T, linearmath::qualifier Q>
	struct hash<linearmath::mat<4, 4, T, Q> >
	{
		LINEARMATH_FUNC_DECL size_t operator()(linearmath::mat<4, 4, T,Q> const& m) const LINEARMATH_NOEXCEPT;
	};
} // namespace std

#include "hash.inl"

#endif //LINEARMATH_LANG & LINEARMATH_LANG_CXX11
