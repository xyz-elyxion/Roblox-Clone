/// @ref gtx_hash

namespace linearmath {
namespace detail
{
	LINEARMATH_INLINE void hash_combine(size_t &seed, size_t hash)
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}
}}

namespace std
{
	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::vec<1, T, Q> >::operator()(linearmath::vec<1, T, Q> const& v) const LINEARMATH_NOEXCEPT
	{
		hash<T> hasher;
		return hasher(v.x);
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::vec<2, T, Q> >::operator()(linearmath::vec<2, T, Q> const& v) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		linearmath::detail::hash_combine(seed, hasher(v.x));
		linearmath::detail::hash_combine(seed, hasher(v.y));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::vec<3, T, Q> >::operator()(linearmath::vec<3, T, Q> const& v) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		linearmath::detail::hash_combine(seed, hasher(v.x));
		linearmath::detail::hash_combine(seed, hasher(v.y));
		linearmath::detail::hash_combine(seed, hasher(v.z));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::vec<4, T, Q> >::operator()(linearmath::vec<4, T, Q> const& v) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		linearmath::detail::hash_combine(seed, hasher(v.x));
		linearmath::detail::hash_combine(seed, hasher(v.y));
		linearmath::detail::hash_combine(seed, hasher(v.z));
		linearmath::detail::hash_combine(seed, hasher(v.w));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::qua<T, Q> >::operator()(linearmath::qua<T,Q> const& q) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		linearmath::detail::hash_combine(seed, hasher(q.x));
		linearmath::detail::hash_combine(seed, hasher(q.y));
		linearmath::detail::hash_combine(seed, hasher(q.z));
		linearmath::detail::hash_combine(seed, hasher(q.w));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::tdualquat<T, Q> >::operator()(linearmath::tdualquat<T, Q> const& q) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::qua<T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(q.real));
		linearmath::detail::hash_combine(seed, hasher(q.dual));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<2, 2, T, Q> >::operator()(linearmath::mat<2, 2, T, Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<2, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<2, 3, T, Q> >::operator()(linearmath::mat<2, 3, T, Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<3, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<2, 4, T, Q> >::operator()(linearmath::mat<2, 4, T, Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<4, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<3, 2, T, Q> >::operator()(linearmath::mat<3, 2, T, Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<2, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		linearmath::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<3, 3, T, Q> >::operator()(linearmath::mat<3, 3, T, Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<3, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		linearmath::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<3, 4, T, Q> >::operator()(linearmath::mat<3, 4, T, Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<4, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		linearmath::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<4, 2, T,Q> >::operator()(linearmath::mat<4, 2, T,Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<2, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		linearmath::detail::hash_combine(seed, hasher(m[2]));
		linearmath::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<4, 3, T,Q> >::operator()(linearmath::mat<4, 3, T,Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<3, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		linearmath::detail::hash_combine(seed, hasher(m[2]));
		linearmath::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}

	template<typename T, linearmath::qualifier Q>
	LINEARMATH_FUNC_QUALIFIER size_t hash<linearmath::mat<4, 4, T,Q> >::operator()(linearmath::mat<4, 4, T, Q> const& m) const LINEARMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<linearmath::vec<4, T, Q> > hasher;
		linearmath::detail::hash_combine(seed, hasher(m[0]));
		linearmath::detail::hash_combine(seed, hasher(m[1]));
		linearmath::detail::hash_combine(seed, hasher(m[2]));
		linearmath::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}
}
