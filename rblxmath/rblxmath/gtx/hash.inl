/// @ref gtx_hash

namespace rblxmath {
namespace detail
{
	RBLXMATH_INLINE void hash_combine(size_t &seed, size_t hash)
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}
}}

namespace std
{
	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::vec<1, T, Q> >::operator()(rblxmath::vec<1, T, Q> const& v) const RBLXMATH_NOEXCEPT
	{
		hash<T> hasher;
		return hasher(v.x);
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::vec<2, T, Q> >::operator()(rblxmath::vec<2, T, Q> const& v) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		rblxmath::detail::hash_combine(seed, hasher(v.x));
		rblxmath::detail::hash_combine(seed, hasher(v.y));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::vec<3, T, Q> >::operator()(rblxmath::vec<3, T, Q> const& v) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		rblxmath::detail::hash_combine(seed, hasher(v.x));
		rblxmath::detail::hash_combine(seed, hasher(v.y));
		rblxmath::detail::hash_combine(seed, hasher(v.z));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::vec<4, T, Q> >::operator()(rblxmath::vec<4, T, Q> const& v) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		rblxmath::detail::hash_combine(seed, hasher(v.x));
		rblxmath::detail::hash_combine(seed, hasher(v.y));
		rblxmath::detail::hash_combine(seed, hasher(v.z));
		rblxmath::detail::hash_combine(seed, hasher(v.w));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::qua<T, Q> >::operator()(rblxmath::qua<T,Q> const& q) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<T> hasher;
		rblxmath::detail::hash_combine(seed, hasher(q.x));
		rblxmath::detail::hash_combine(seed, hasher(q.y));
		rblxmath::detail::hash_combine(seed, hasher(q.z));
		rblxmath::detail::hash_combine(seed, hasher(q.w));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::tdualquat<T, Q> >::operator()(rblxmath::tdualquat<T, Q> const& q) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::qua<T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(q.real));
		rblxmath::detail::hash_combine(seed, hasher(q.dual));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<2, 2, T, Q> >::operator()(rblxmath::mat<2, 2, T, Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<2, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<2, 3, T, Q> >::operator()(rblxmath::mat<2, 3, T, Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<3, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<2, 4, T, Q> >::operator()(rblxmath::mat<2, 4, T, Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<4, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<3, 2, T, Q> >::operator()(rblxmath::mat<3, 2, T, Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<2, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		rblxmath::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<3, 3, T, Q> >::operator()(rblxmath::mat<3, 3, T, Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<3, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		rblxmath::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<3, 4, T, Q> >::operator()(rblxmath::mat<3, 4, T, Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<4, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		rblxmath::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<4, 2, T,Q> >::operator()(rblxmath::mat<4, 2, T,Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<2, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		rblxmath::detail::hash_combine(seed, hasher(m[2]));
		rblxmath::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<4, 3, T,Q> >::operator()(rblxmath::mat<4, 3, T,Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<3, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		rblxmath::detail::hash_combine(seed, hasher(m[2]));
		rblxmath::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}

	template<typename T, rblxmath::qualifier Q>
	RBLXMATH_FUNC_QUALIFIER size_t hash<rblxmath::mat<4, 4, T,Q> >::operator()(rblxmath::mat<4, 4, T, Q> const& m) const RBLXMATH_NOEXCEPT
	{
		size_t seed = 0;
		hash<rblxmath::vec<4, T, Q> > hasher;
		rblxmath::detail::hash_combine(seed, hasher(m[0]));
		rblxmath::detail::hash_combine(seed, hasher(m[1]));
		rblxmath::detail::hash_combine(seed, hasher(m[2]));
		rblxmath::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}
}
