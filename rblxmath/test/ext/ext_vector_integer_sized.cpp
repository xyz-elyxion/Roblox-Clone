#include <rblxmath/ext/vector_integer.hpp>
#include <rblxmath/ext/vector_int1.hpp>
#include <rblxmath/ext/vector_int1_sized.hpp>
#include <rblxmath/ext/vector_uint1.hpp>
#include <rblxmath/ext/vector_uint1_sized.hpp>

template <typename genType>
static int test_operators()
{
	int Error = 0;

	{
		genType const A(1);
		genType const B(1);

		bool const R = A != B;
		bool const S = A == B;
		Error += (S && !R) ? 0 : 1;
	}

	{
		genType const A(1);
		genType const B(1);

		genType const C = A + B;
		Error += C == genType(2) ? 0 : 1;

		genType const D = A - B;
		Error += D == genType(0) ? 0 : 1;

		genType const E = A * B;
		Error += E == genType(1) ? 0 : 1;

		genType const F = A / B;
		Error += F == genType(1) ? 0 : 1;
	}

	{
		genType const A(3);
		genType const B(2);

		genType const C = A % B;
		Error += C == genType(1) ? 0 : 1;
	}

	{
		genType const A(1);
		genType const B(1);
		genType const C(0);

		genType const I = A & B;
		Error += I == genType(1) ? 0 : 1;
		genType const D = A & C;
		Error += D == genType(0) ? 0 : 1;

		genType const E = A | B;
		Error += E == genType(1) ? 0 : 1;
		genType const F = A | C;
		Error += F == genType(1) ? 0 : 1;

		genType const G = A ^ B;
		Error += G == genType(0) ? 0 : 1;
		genType const H = A ^ C;
		Error += H == genType(1) ? 0 : 1;
	}

	{
		genType B(1);
		genType C(2);

		genType D = B << B;
		Error += D == genType(2) ? 0 : 1;
		genType E = C >> B;
		Error += E == genType(1) ? 0 : 1;
	}

	return Error;
}

template <typename genType>
static int test_ctor()
{
	typedef typename genType::value_type T;
	
	int Error = 0;

	genType const A = genType(1);

	genType const E(genType(1));
	Error += A == E ? 0 : 1;

	genType const F(E);
	Error += A == F ? 0 : 1;

	genType const B = genType(1);
	genType const G(rblxmath::vec<2, T>(1));
	Error += B == G ? 0 : 1;

	genType const H(rblxmath::vec<3, T>(1));
	Error += B == H ? 0 : 1;

	genType const I(rblxmath::vec<4, T>(1));
	Error += B == I ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_size()
{
	int Error = 0;

	Error += sizeof(typename genType::value_type) == sizeof(genType) ? 0 : 1;
	Error += genType().length() == 1 ? 0 : 1;
	Error += genType::length() == 1 ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_relational()
{
	int Error = 0;

	genType const A(1);
	genType const B(1);
	genType const C(0);

	Error += A == B ? 0 : 1;
	Error += A != C ? 0 : 1;
	Error += all(equal(A, B)) ? 0 : 1;
	Error += any(notEqual(A, C)) ? 0 : 1;

	return Error;
}

template <typename genType>
static int test_constexpr()
{
#	if RBLXMATH_CONFIG_CONSTEXP == RBLXMATH_ENABLE
		static_assert(genType::length() == 1, "RBLXMATH: Failed constexpr");
		static_assert(genType(1)[0] == 1, "RBLXMATH: Failed constexpr");
		static_assert(genType(1) == genType(1), "RBLXMATH: Failed constexpr");
		static_assert(genType(1) != genType(0), "RBLXMATH: Failed constexpr");
#	endif

	return 0;
}

int main()
{
	int Error = 0;

	Error += test_operators<rblxmath::ivec1>();
	Error += test_operators<rblxmath::i8vec1>();
	Error += test_operators<rblxmath::i16vec1>();
	Error += test_operators<rblxmath::i32vec1>();
	Error += test_operators<rblxmath::i64vec1>();

	Error += test_ctor<rblxmath::ivec1>();
	Error += test_ctor<rblxmath::i8vec1>();
	Error += test_ctor<rblxmath::i16vec1>();
	Error += test_ctor<rblxmath::i32vec1>();
	Error += test_ctor<rblxmath::i64vec1>();

	Error += test_size<rblxmath::ivec1>();
	Error += test_size<rblxmath::i8vec1>();
	Error += test_size<rblxmath::i16vec1>();
	Error += test_size<rblxmath::i32vec1>();
	Error += test_size<rblxmath::i64vec1>();

	Error += test_relational<rblxmath::ivec1>();
	Error += test_relational<rblxmath::i8vec1>();
	Error += test_relational<rblxmath::i16vec1>();
	Error += test_relational<rblxmath::i32vec1>();
	Error += test_relational<rblxmath::i64vec1>();

	Error += test_constexpr<rblxmath::ivec1>();
	Error += test_constexpr<rblxmath::i8vec1>();
	Error += test_constexpr<rblxmath::i16vec1>();
	Error += test_constexpr<rblxmath::i32vec1>();
	Error += test_constexpr<rblxmath::i64vec1>();

	Error += test_operators<rblxmath::uvec1>();
	Error += test_operators<rblxmath::u8vec1>();
	Error += test_operators<rblxmath::u16vec1>();
	Error += test_operators<rblxmath::u32vec1>();
	Error += test_operators<rblxmath::u64vec1>();
	
	Error += test_ctor<rblxmath::uvec1>();
	Error += test_ctor<rblxmath::u8vec1>();
	Error += test_ctor<rblxmath::u16vec1>();
	Error += test_ctor<rblxmath::u32vec1>();
	Error += test_ctor<rblxmath::u64vec1>();
	
	Error += test_size<rblxmath::uvec1>();
	Error += test_size<rblxmath::u8vec1>();
	Error += test_size<rblxmath::u16vec1>();
	Error += test_size<rblxmath::u32vec1>();
	Error += test_size<rblxmath::u64vec1>();
	
	Error += test_relational<rblxmath::uvec1>();
	Error += test_relational<rblxmath::u8vec1>();
	Error += test_relational<rblxmath::u16vec1>();
	Error += test_relational<rblxmath::u32vec1>();
	Error += test_relational<rblxmath::u64vec1>();
	
	Error += test_constexpr<rblxmath::uvec1>();
	Error += test_constexpr<rblxmath::u8vec1>();
	Error += test_constexpr<rblxmath::u16vec1>();
	Error += test_constexpr<rblxmath::u32vec1>();
	Error += test_constexpr<rblxmath::u64vec1>();
	
	return Error;
}
