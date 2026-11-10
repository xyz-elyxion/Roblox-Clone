namespace rblxmath
{
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR vec<L, bool, Q> lessThan(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		vec<L, bool, Q> Result(true);
		for(length_t i = 0; i < L; ++i)
			Result[i] = x[i] < y[i];
		return Result;
	}

	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR vec<L, bool, Q> lessThanEqual(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		vec<L, bool, Q> Result(true);
		for(length_t i = 0; i < L; ++i)
			Result[i] = x[i] <= y[i];
		return Result;
	}

	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR vec<L, bool, Q> greaterThan(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		vec<L, bool, Q> Result(true);
		for(length_t i = 0; i < L; ++i)
			Result[i] = x[i] > y[i];
		return Result;
	}

	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR vec<L, bool, Q> greaterThanEqual(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		vec<L, bool, Q> Result(true);
		for(length_t i = 0; i < L; ++i)
			Result[i] = x[i] >= y[i];
		return Result;
	}

	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR vec<L, bool, Q> equal(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		vec<L, bool, Q> Result(true);
		for(length_t i = 0; i < L; ++i)
			Result[i] = x[i] == y[i];
		return Result;
	}

	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR vec<L, bool, Q> notEqual(vec<L, T, Q> const& x, vec<L, T, Q> const& y)
	{
		vec<L, bool, Q> Result(true);
		for(length_t i = 0; i < L; ++i)
			Result[i] = x[i] != y[i];
		return Result;
	}

	template<length_t L, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR bool any(vec<L, bool, Q> const& v)
	{
		bool Result = false;
		for(length_t i = 0; i < L; ++i)
			Result = Result || v[i];
		return Result;
	}

	template<length_t L, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR bool all(vec<L, bool, Q> const& v)
	{
		bool Result = true;
		for(length_t i = 0; i < L; ++i)
			Result = Result && v[i];
		return Result;
	}

	template<length_t L, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR vec<L, bool, Q> not_(vec<L, bool, Q> const& v)
	{
		vec<L, bool, Q> Result(true);
		for(length_t i = 0; i < L; ++i)
			Result[i] = !v[i];
		return Result;
	}
}//namespace rblxmath

#if RBLXMATH_CONFIG_SIMD == RBLXMATH_ENABLE
#	include "func_vector_relational_simd.inl"
#endif
