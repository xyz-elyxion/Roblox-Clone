/// @ref gtx_extended_min_max

namespace rblxmath
{
	template<typename T>
	RBLXMATH_FUNC_QUALIFIER T min(
		T const& x,
		T const& y,
		T const& z)
	{
		return rblxmath::min(rblxmath::min(x, y), z);
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z
	)
	{
		return rblxmath::min(rblxmath::min(x, y), z);
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z
	)
	{
		return rblxmath::min(rblxmath::min(x, y), z);
	}

	template<typename T>
	RBLXMATH_FUNC_QUALIFIER T min
	(
		T const& x,
		T const& y,
		T const& z,
		T const& w
	)
	{
		return rblxmath::min(rblxmath::min(x, y), rblxmath::min(z, w));
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z,
		typename C<T>::T const& w
	)
	{
		return rblxmath::min(rblxmath::min(x, y), rblxmath::min(z, w));
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z,
		C<T> const& w
	)
	{
		return rblxmath::min(rblxmath::min(x, y), rblxmath::min(z, w));
	}

	template<typename T>
	RBLXMATH_FUNC_QUALIFIER T max(
		T const& x,
		T const& y,
		T const& z)
	{
		return rblxmath::max(rblxmath::max(x, y), z);
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z
	)
	{
		return rblxmath::max(rblxmath::max(x, y), z);
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z
	)
	{
		return rblxmath::max(rblxmath::max(x, y), z);
	}

	template<typename T>
	RBLXMATH_FUNC_QUALIFIER T max
	(
		T const& x,
		T const& y,
		T const& z,
		T const& w
	)
	{
		return rblxmath::max(rblxmath::max(x, y), rblxmath::max(z, w));
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z,
		typename C<T>::T const& w
	)
	{
		return rblxmath::max(rblxmath::max(x, y), rblxmath::max(z, w));
	}

	template<typename T, template<typename> class C>
	RBLXMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z,
		C<T> const& w
	)
	{
		return rblxmath::max(rblxmath::max(x, y), rblxmath::max(z, w));
	}
}//namespace rblxmath
