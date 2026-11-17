/// @ref gtx_extended_min_max

namespace linearmath
{
	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T min(
		T const& x,
		T const& y,
		T const& z)
	{
		return linearmath::min(linearmath::min(x, y), z);
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z
	)
	{
		return linearmath::min(linearmath::min(x, y), z);
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z
	)
	{
		return linearmath::min(linearmath::min(x, y), z);
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T min
	(
		T const& x,
		T const& y,
		T const& z,
		T const& w
	)
	{
		return linearmath::min(linearmath::min(x, y), linearmath::min(z, w));
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z,
		typename C<T>::T const& w
	)
	{
		return linearmath::min(linearmath::min(x, y), linearmath::min(z, w));
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> min
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z,
		C<T> const& w
	)
	{
		return linearmath::min(linearmath::min(x, y), linearmath::min(z, w));
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T max(
		T const& x,
		T const& y,
		T const& z)
	{
		return linearmath::max(linearmath::max(x, y), z);
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z
	)
	{
		return linearmath::max(linearmath::max(x, y), z);
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z
	)
	{
		return linearmath::max(linearmath::max(x, y), z);
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T max
	(
		T const& x,
		T const& y,
		T const& z,
		T const& w
	)
	{
		return linearmath::max(linearmath::max(x, y), linearmath::max(z, w));
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		typename C<T>::T const& y,
		typename C<T>::T const& z,
		typename C<T>::T const& w
	)
	{
		return linearmath::max(linearmath::max(x, y), linearmath::max(z, w));
	}

	template<typename T, template<typename> class C>
	LINEARMATH_FUNC_QUALIFIER C<T> max
	(
		C<T> const& x,
		C<T> const& y,
		C<T> const& z,
		C<T> const& w
	)
	{
		return linearmath::max(linearmath::max(x, y), linearmath::max(z, w));
	}
}//namespace linearmath
