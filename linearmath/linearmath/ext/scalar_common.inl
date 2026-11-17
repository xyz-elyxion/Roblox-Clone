namespace linearmath
{
	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T min(T a, T b, T c)
	{
		return linearmath::min(linearmath::min(a, b), c);
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T min(T a, T b, T c, T d)
	{
		return linearmath::min(linearmath::min(a, b), linearmath::min(c, d));
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T max(T a, T b, T c)
	{
		return linearmath::max(linearmath::max(a, b), c);
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T max(T a, T b, T c, T d)
	{
		return linearmath::max(linearmath::max(a, b), linearmath::max(c, d));
	}

#	if LINEARMATH_HAS_CXX11_STL
		using std::fmin;
#	else
		template<typename T>
		LINEARMATH_FUNC_QUALIFIER T fmin(T a, T b)
		{
			LINEARMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'fmin' only accept floating-point input");

			if (isnan(a))
				return b;
			return min(a, b);
		}
#	endif

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T fmin(T a, T b, T c)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'fmin' only accept floating-point input");

		if (isnan(a))
			return fmin(b, c);
		if (isnan(b))
			return fmin(a, c);
		if (isnan(c))
			return min(a, b);
		return min(a, b, c);
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T fmin(T a, T b, T c, T d)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'fmin' only accept floating-point input");

		if (isnan(a))
			return fmin(b, c, d);
		if (isnan(b))
			return min(a, fmin(c, d));
		if (isnan(c))
			return fmin(min(a, b), d);
		if (isnan(d))
			return min(a, b, c);
		return min(a, b, c, d);
	}


#	if LINEARMATH_HAS_CXX11_STL
		using std::fmax;
#	else
		template<typename T>
		LINEARMATH_FUNC_QUALIFIER T fmax(T a, T b)
		{
			LINEARMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'fmax' only accept floating-point input");

			if (isnan(a))
				return b;
			return max(a, b);
		}
#	endif

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T fmax(T a, T b, T c)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'fmax' only accept floating-point input");

		if (isnan(a))
			return fmax(b, c);
		if (isnan(b))
			return fmax(a, c);
		if (isnan(c))
			return max(a, b);
		return max(a, b, c);
	}

	template<typename T>
	LINEARMATH_FUNC_QUALIFIER T fmax(T a, T b, T c, T d)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<T>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'fmax' only accept floating-point input");

		if (isnan(a))
			return fmax(b, c, d);
		if (isnan(b))
			return max(a, fmax(c, d));
		if (isnan(c))
			return fmax(max(a, b), d);
		if (isnan(d))
			return max(a, b, c);
		return max(a, b, c, d);
	}

	// fclamp
	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType fclamp(genType x, genType minVal, genType maxVal)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'fclamp' only accept floating-point or integer inputs");
		return fmin(fmax(x, minVal), maxVal);
	}

	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType clamp(genType const& Texcoord)
	{
		return linearmath::clamp(Texcoord, static_cast<genType>(0), static_cast<genType>(1));
	}

	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType repeat(genType const& Texcoord)
	{
		return linearmath::fract(Texcoord);
	}

	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType mirrorClamp(genType const& Texcoord)
	{
		return linearmath::fract(linearmath::abs(Texcoord));
	}

	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER genType mirrorRepeat(genType const& Texcoord)
	{
		genType const Abs = linearmath::abs(Texcoord);
		genType const Clamp = linearmath::mod(linearmath::floor(Abs), static_cast<genType>(2));
		genType const Floor = linearmath::floor(Abs);
		genType const Rest = Abs - Floor;
		genType const Mirror = Clamp + Rest;
		return mix(Rest, static_cast<genType>(1) - Rest, Mirror >= static_cast<genType>(1));
	}

	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER int iround(genType const& x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'iround' only accept floating-point inputs");
		assert(static_cast<genType>(0.0) <= x);

		return static_cast<int>(x + static_cast<genType>(0.5));
	}

	template<typename genType>
	LINEARMATH_FUNC_QUALIFIER uint uround(genType const& x)
	{
		LINEARMATH_STATIC_ASSERT(std::numeric_limits<genType>::is_iec559 || LINEARMATH_CONFIG_UNRESTRICTED_FLOAT, "'uround' only accept floating-point inputs");
		assert(static_cast<genType>(0.0) <= x);

		return static_cast<uint>(x + static_cast<genType>(0.5));
	}
}//namespace linearmath
