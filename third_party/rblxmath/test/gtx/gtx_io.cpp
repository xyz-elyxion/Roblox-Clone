#include <rblxmath/rblxmath.hpp>
#if RBLXMATH_LANG & RBLXMATH_LANG_CXXMS_FLAG
#include <rblxmath/gtc/type_precision.hpp>
#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/io.hpp>
#include <iostream>
#include <sstream>
#include <typeinfo>

namespace
{
	template<typename CTy, typename CTr>
	static std::basic_ostream<CTy,CTr>& operator<<(std::basic_ostream<CTy,CTr>& os, rblxmath::qualifier const& a)
	{
		typename std::basic_ostream<CTy,CTr>::sentry const cerberus(os);

		if (cerberus)
		{
			switch (a) {
			case rblxmath::highp:			os << "uhi"; break;
			case rblxmath::mediump:			os << "umd"; break;
			case rblxmath::lowp:				os << "ulo"; break;
#			if RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_ENABLE
				case rblxmath::aligned_highp:	os << "ahi"; break;
				case rblxmath::aligned_mediump:	os << "amd"; break;
				case rblxmath::aligned_lowp:		os << "alo"; break;
#			endif
			}
		}

		return os;
	}

	template<typename U, rblxmath::qualifier P, typename T, typename CTy, typename CTr>
	static std::basic_string<CTy> type_name(std::basic_ostream<CTy,CTr>&, T const&)
	{
		std::basic_ostringstream<CTy,CTr> ostr;

		if      (typeid(T) == typeid(rblxmath::qua<U,P>))   { ostr << "quat"; }
		else if (typeid(T) == typeid(rblxmath::vec<2, U,P>))   { ostr << "vec2"; }
		else if (typeid(T) == typeid(rblxmath::vec<3, U,P>))   { ostr << "vec3"; }
		else if (typeid(T) == typeid(rblxmath::vec<4, U,P>))   { ostr << "vec4"; }
		else if (typeid(T) == typeid(rblxmath::mat<2, 2, U,P>)) { ostr << "mat2x2"; }
		else if (typeid(T) == typeid(rblxmath::mat<2, 3, U,P>)) { ostr << "mat2x3"; }
		else if (typeid(T) == typeid(rblxmath::mat<2, 4, U,P>)) { ostr << "mat2x4"; }
		else if (typeid(T) == typeid(rblxmath::mat<3, 2, U,P>)) { ostr << "mat3x2"; }
		else if (typeid(T) == typeid(rblxmath::mat<3, 3, U,P>)) { ostr << "mat3x3"; }
		else if (typeid(T) == typeid(rblxmath::mat<3, 4, U,P>)) { ostr << "mat3x4"; }
		else if (typeid(T) == typeid(rblxmath::mat<4, 2, U,P>)) { ostr << "mat4x2"; }
		else if (typeid(T) == typeid(rblxmath::mat<4, 3, U,P>)) { ostr << "mat4x3"; }
		else if (typeid(T) == typeid(rblxmath::mat<4, 4, U,P>)) { ostr << "mat4x4"; }
		else                                             { ostr << "unknown"; }

		ostr << '<' << typeid(U).name() << ',' << P << '>';

		return ostr.str();
	}
} // namespace {

template<typename T, rblxmath::qualifier P, typename OS>
static int test_io_quat(OS& os)
{
	os << '\n' << typeid(OS).name() << '\n';

	rblxmath::qua<T, P> const q(1, 0, 0, 0);

	{
		rblxmath::io::basic_format_saver<typename OS::char_type> const iofs(os);

		os << rblxmath::io::precision(2) << rblxmath::io::width(1 + 2 + 1 + 2)
			<< type_name<T, P>(os, q) << ": " << q << '\n';
	}

	{
		rblxmath::io::basic_format_saver<typename OS::char_type> const iofs(os);

		os << rblxmath::io::unformatted
			<< type_name<T, P>(os, q) << ": " << q << '\n';
	}

	return 0;
}

template<typename T, rblxmath::qualifier P, typename OS>
static int test_io_vec(OS& os)
{
	os << '\n' << typeid(OS).name() << '\n';

	rblxmath::vec<2, T,P> const v2(0, 1);
	rblxmath::vec<3, T,P> const v3(2, 3, 4);
	rblxmath::vec<4, T,P> const v4(5, 6, 7, 8);

	os << type_name<T,P>(os, v2) << ": " << v2 << '\n'
		<< type_name<T,P>(os, v3) << ": " << v3 << '\n'
		<< type_name<T,P>(os, v4) << ": " << v4 << '\n';

	rblxmath::io::basic_format_saver<typename OS::char_type> const iofs(os);

	os << rblxmath::io::precision(2) << rblxmath::io::width(1 + 2 + 1 + 2)
		<< type_name<T,P>(os, v2) << ": " << v2 << '\n'
		<< type_name<T,P>(os, v3) << ": " << v3 << '\n'
		<< type_name<T,P>(os, v4) << ": " << v4 << '\n';

	return 0;
}

template<typename T, rblxmath::qualifier P, typename OS>
static int test_io_mat(OS& os, rblxmath::io::order_type otype)
{
	os << '\n' << typeid(OS).name() << '\n';

	rblxmath::vec<2, T,P> const v2_1( 0,  1);
	rblxmath::vec<2, T,P> const v2_2( 2,  3);
	rblxmath::vec<2, T,P> const v2_3( 4,  5);
	rblxmath::vec<2, T,P> const v2_4( 6,  7);
	rblxmath::vec<3, T,P> const v3_1( 8,  9, 10);
	rblxmath::vec<3, T,P> const v3_2(11, 12, 13);
	rblxmath::vec<3, T,P> const v3_3(14, 15, 16);
	rblxmath::vec<3, T,P> const v3_4(17, 18, 19);
	rblxmath::vec<4, T,P> const v4_1(20, 21, 22, 23);
	rblxmath::vec<4, T,P> const v4_2(24, 25, 26, 27);
	rblxmath::vec<4, T,P> const v4_3(28, 29, 30, 31);
	rblxmath::vec<4, T,P> const v4_4(32, 33, 34, 35);

	rblxmath::io::basic_format_saver<typename OS::char_type> const iofs(os);

	os << rblxmath::io::precision(2) << rblxmath::io::width(1 + 2 + 1 + 2)
		<< rblxmath::io::order(otype)
		<< "mat2x2<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<2, 2, T,P>(v2_1, v2_2) << '\n'
		<< "mat2x3<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<2, 3, T,P>(v3_1, v3_2) << '\n'
		<< "mat2x4<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<2, 4, T,P>(v4_1, v4_2) << '\n'
		<< "mat3x2<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<3, 2, T,P>(v2_1, v2_2, v2_3) << '\n'
		<< "mat3x3<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<3, 3, T,P>(v3_1, v3_2, v3_3) << '\n'
		<< "mat3x4<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<3, 4, T,P>(v4_1, v4_2, v4_3) << '\n'
		<< "mat4x2<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<4, 2, T,P>(v2_1, v2_2, v2_3, v2_4) << '\n'
		<< "mat4x3<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<4, 3, T,P>(v3_1, v3_2, v3_3, v3_4) << '\n'
		<< "mat4x4<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<4, 4, T,P>(v4_1, v4_2, v4_3, v4_4) << '\n';

	os << rblxmath::io::unformatted
		<< rblxmath::io::order(otype)
		<< "mat2x2<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<2, 2, T,P>(v2_1, v2_2) << '\n'
		<< "mat2x3<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<2, 3, T,P>(v3_1, v3_2) << '\n'
		<< "mat2x4<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<2, 4, T,P>(v4_1, v4_2) << '\n'
		<< "mat3x2<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<3, 2, T,P>(v2_1, v2_2, v2_3) << '\n'
		<< "mat3x3<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<3, 3, T,P>(v3_1, v3_2, v3_3) << '\n'
		<< "mat3x4<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<3, 4, T,P>(v4_1, v4_2, v4_3) << '\n'
		<< "mat4x2<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<4, 2, T,P>(v2_1, v2_2, v2_3, v2_4) << '\n'
		<< "mat4x3<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<4, 3, T,P>(v3_1, v3_2, v3_3, v3_4) << '\n'
		<< "mat4x4<" << typeid(T).name() << ',' << P << ">: " << rblxmath::mat<4, 4, T,P>(v4_1, v4_2, v4_3, v4_4) << '\n';
  
	return 0;
}

int main()
{
	int Error(0);

	Error += test_io_quat<float, rblxmath::highp>(std::cout);
	Error += test_io_quat<float, rblxmath::highp>(std::wcout);
	Error += test_io_quat<int, rblxmath::mediump>(std::cout);
	Error += test_io_quat<int, rblxmath::mediump>(std::wcout);
	Error += test_io_quat<rblxmath::uint, rblxmath::lowp>(std::cout);
	Error += test_io_quat<rblxmath::uint, rblxmath::lowp>(std::wcout);

	Error += test_io_vec<float, rblxmath::highp>(std::cout);
	Error += test_io_vec<float, rblxmath::highp>(std::wcout);
	Error += test_io_vec<int, rblxmath::mediump>(std::cout);
	Error += test_io_vec<int, rblxmath::mediump>(std::wcout);
	Error += test_io_vec<rblxmath::uint, rblxmath::lowp>(std::cout);
	Error += test_io_vec<rblxmath::uint, rblxmath::lowp>(std::wcout);

	Error += test_io_mat<float, rblxmath::highp>(std::cout, rblxmath::io::column_major);
	Error += test_io_mat<float, rblxmath::lowp>(std::wcout, rblxmath::io::column_major);
	Error += test_io_mat<float, rblxmath::highp>(std::cout, rblxmath::io::row_major);
        Error += test_io_mat<float, rblxmath::lowp>(std::wcout, rblxmath::io::row_major);

	return Error;
}
#else

int main()
{
	return 0;
}

#endif// RBLXMATH_LANG & RBLXMATH_LANG_CXXMS_FLAG
