#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/ext/scalar_relational.hpp>
#include <rblxmath/ext/vector_relational.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/matrix.hpp>
#include <rblxmath/mat4x4.hpp>
#include <rblxmath/vec4.hpp>
#include <vector>

template <typename matType, typename vecType>
static int test_operators()
{
	typedef typename matType::value_type value_type;

	value_type const Epsilon = static_cast<value_type>(0.001);

	int Error = 0;

	matType const M(static_cast<value_type>(2.0f));
	matType const N(static_cast<value_type>(1.0f));
	vecType const U(static_cast<value_type>(2.0f));

	{
		matType const P = N * static_cast<value_type>(2.0f);
		Error += rblxmath::all(rblxmath::equal(P, M, Epsilon)) ? 0 : 1;
		
		matType const Q = M / static_cast<value_type>(2.0f);
		Error += rblxmath::all(rblxmath::equal(Q, N, Epsilon)) ? 0 : 1;
	}
	
	{
		vecType const V = M * U;
		Error += rblxmath::all(rblxmath::equal(V, vecType(static_cast<value_type>(4.f)), Epsilon)) ? 0 : 1;
		
		vecType const W = U / M;
		Error += rblxmath::all(rblxmath::equal(W, vecType(static_cast<value_type>(1.f)), Epsilon)) ? 0 : 1;
	}

	{
		matType const O = M * N;
		Error += rblxmath::all(rblxmath::equal(O, matType(static_cast<value_type>(2.f)), Epsilon)) ? 0 : 1;
	}

	return Error;
}

template <typename matType>
static int test_inverse()
{
	typedef typename matType::value_type value_type;

	value_type const Epsilon = static_cast<value_type>(0.001);
	
	int Error = 0;

	matType const Identity(static_cast<value_type>(1.0f));
	matType const Matrix(
		rblxmath::vec4(0.6f, 0.2f, 0.3f, 0.4f),
		rblxmath::vec4(0.2f, 0.7f, 0.5f, 0.3f),
		rblxmath::vec4(0.3f, 0.5f, 0.7f, 0.2f),
		rblxmath::vec4(0.4f, 0.3f, 0.2f, 0.6f));
	matType const Inverse = Identity / Matrix;
	matType const Result = Matrix * Inverse;

	Error += rblxmath::all(rblxmath::equal(Identity, Result, Epsilon)) ? 0 : 1;
	
	return Error;
}

static int test_ctr()
{
	int Error = 0;

#if RBLXMATH_HAS_TRIVIAL_QUERIES
	//Error += std::is_trivially_default_constructible<rblxmath::mat4>::value ? 0 : 1;
	//Error += std::is_trivially_copy_assignable<rblxmath::mat4>::value ? 0 : 1;
	Error += std::is_trivially_copyable<rblxmath::mat4>::value ? 0 : 1;
	//Error += std::is_copy_constructible<rblxmath::mat4>::value ? 0 : 1;
	//Error += std::has_trivial_copy_constructor<rblxmath::mat4>::value ? 0 : 1;
#endif

#if RBLXMATH_HAS_INITIALIZER_LISTS
	rblxmath::mat4 const m0(
		rblxmath::vec4(0, 1, 2, 3),
		rblxmath::vec4(4, 5, 6, 7),
		rblxmath::vec4(8, 9, 10, 11),
		rblxmath::vec4(12, 13, 14, 15));

	assert(sizeof(m0) == 4 * 4 * 4);

	rblxmath::vec4 const V{0, 1, 2, 3};

	rblxmath::mat4 const m1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

	rblxmath::mat4 const m2{
		V,
		{4, 5, 6, 7},
		{8, 9, 10, 11},
		{12, 13, 14, 15}};

	Error += rblxmath::all(rblxmath::equal(m0, m2, rblxmath::epsilon<float>())) ? 0 : 1;
	Error += rblxmath::all(rblxmath::equal(m1, m2, rblxmath::epsilon<float>())) ? 0 : 1;


	std::vector<rblxmath::mat4> const m3{
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

	rblxmath::mat4 const m4{
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1} };

	Error += rblxmath::equal(m4[0][0], 1.0f, 0.0001f) ? 0 : 1;
	Error += rblxmath::equal(m4[3][3], 1.0f, 0.0001f) ? 0 : 1;

	std::vector<rblxmath::mat4> const v1{
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

	std::vector<rblxmath::mat4> const v2{
		{
			{ 0, 1, 2, 3 },
			{ 4, 5, 6, 7 },
			{ 8, 9, 10, 11 },
			{ 12, 13, 14, 15 }
		},
		{
			{ 0, 1, 2, 3 },
			{ 4, 5, 6, 7 },
			{ 8, 9, 10, 11 },
			{ 12, 13, 14, 15 }
		}};

#endif//RBLXMATH_HAS_INITIALIZER_LISTS

	return Error;
}

static int test_member_alloc_bug()
{
	int Error = 0;
	
	struct repro
	{
		repro(){ this->matrix = new rblxmath::mat4(); }
		~repro(){delete this->matrix;}
		
		rblxmath::mat4* matrix;
	};
	
	repro Repro;
	
	return Error;
}

static int test_size()
{
	int Error = 0;

	Error += 64 == sizeof(rblxmath::mat4) ? 0 : 1;
	Error += 128 == sizeof(rblxmath::dmat4) ? 0 : 1;
	Error += rblxmath::mat4().length() == 4 ? 0 : 1;
	Error += rblxmath::dmat4().length() == 4 ? 0 : 1;
	Error += rblxmath::mat4::length() == 4 ? 0 : 1;
	Error += rblxmath::dmat4::length() == 4 ? 0 : 1;

	return Error;
}

static int test_constexpr()
{
	int Error = 0;

#if RBLXMATH_HAS_CONSTEXPR
	static_assert(rblxmath::mat4::length() == 4, "RBLXMATH: Failed constexpr");
	constexpr rblxmath::mat4 A(1.f);
	constexpr rblxmath::mat4 B(1.f);
	constexpr rblxmath::bvec4 C = rblxmath::equal(A, B, 0.01f);
	static_assert(rblxmath::all(C), "RBLXMATH: Failed constexpr");

	constexpr rblxmath::mat4 const Z(1.0f);
	Error += rblxmath::all(rblxmath::equal(Z, rblxmath::mat4(1.0f), rblxmath::epsilon<float>())) ? 0 : 1;
#endif

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_member_alloc_bug();
	Error += test_ctr();

	Error += test_operators<rblxmath::mat4, rblxmath::vec4>();
	Error += test_operators<rblxmath::lowp_mat4, rblxmath::lowp_vec4>();
	Error += test_operators<rblxmath::mediump_mat4, rblxmath::mediump_vec4>();
	Error += test_operators<rblxmath::highp_mat4, rblxmath::highp_vec4>();

	Error += test_operators<rblxmath::dmat4, rblxmath::dvec4>();
	Error += test_operators<rblxmath::lowp_dmat4, rblxmath::lowp_dvec4>();
	Error += test_operators<rblxmath::mediump_dmat4, rblxmath::mediump_dvec4>();
	Error += test_operators<rblxmath::highp_dmat4, rblxmath::highp_dvec4>();

	Error += test_inverse<rblxmath::mat4>();
	Error += test_inverse<rblxmath::lowp_mat4>();
	Error += test_inverse<rblxmath::mediump_mat4>();
	Error += test_inverse<rblxmath::highp_mat4>();

	Error += test_inverse<rblxmath::dmat4>();
	Error += test_inverse<rblxmath::lowp_dmat4>();
	Error += test_inverse<rblxmath::mediump_dmat4>();
	Error += test_inverse<rblxmath::highp_dmat4>();

	Error += test_size();
	Error += test_constexpr();

	return Error;
}
