#include <rblxmath/rblxmath.hpp>
/*
#if RBLXMATH_CONFIG_SIMD == RBLXMATH_ENABLE

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/common.hpp>
#include <rblxmath/gtc/integer.hpp>
#include <rblxmath/gtc/epsilon.hpp>
#include <rblxmath/gtc/type_aligned.hpp>
#include <rblxmath/ext/vector_relational.hpp>

namespace rblxmath
{
	enum genTypeEnum
	{
		QUALIFIER_HIGHP,
		QUALIFIER_MEDIUMP,
		QUALIFIER_LOWP
	};

	template <typename genType>
	struct genTypeTrait
	{};

	template <length_t L, typename T>
	struct genTypeTrait<vec<L, T, aligned_highp> >
	{
		static const genTypeEnum GENTYPE = QUALIFIER_HIGHP;
	};

	template <length_t L, typename T>
	struct genTypeTrait<vec<L, T, aligned_mediump> >
	{
		static const genTypeEnum GENTYPE = QUALIFIER_MEDIUMP;
	};

	template <length_t L, typename T>
	struct genTypeTrait<vec<L, T, aligned_lowp> >
	{
		static const genTypeEnum GENTYPE = QUALIFIER_LOWP;
	};

	template<length_t L, typename T, qualifier Q, bool isAligned>
	struct load_gentype
	{
	
	};

#	if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE_BIT
	template<qualifier Q>
	struct load_gentype<4, float, Q, true>
	{
		RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR static vec<4, float, Q> load(float const* Mem)
		{
			vec<4, float, Q> Result;
			Result.data = _mm_loadu_ps(Mem);
			return Result;
		}
	};
#	endif//RBLXMATH_ARCH & RBLXMATH_ARCH_SSE_BIT

	template<typename genType>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR genType example_identity()
	{
		return detail::init_gentype<genType, detail::genTypeTrait<genType>::GENTYPE>::identity();
	}

	template <typename genType, typename valType>
	genType load(valType const* Mem)
	{
		
	}

	aligned_vec4 loadu(float const* Mem)
	{
		aligned_vec4 Result;
#		if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE_BIT
			Result.data = _mm_loadu_ps(Mem);
#		else
			Result[0] = *(Mem + 0);
			Result[1] = *(Mem + 1);
			Result[2] = *(Mem + 2);
			Result[3] = *(Mem + 3);
#		endif//RBLXMATH_ARCH & RBLXMATH_ARCH_SSE_BIT
		return Result;
	}

	aligned_vec4 loada(float const* Mem)
	{
		aligned_vec4 Result;
#		if RBLXMATH_ARCH & RBLXMATH_ARCH_SSE_BIT
			Result.data = _mm_load_ps(Mem);
#		else
			Result[0] = *(Mem + 0);
			Result[1] = *(Mem + 1);
			Result[2] = *(Mem + 2);
			Result[3] = *(Mem + 3);
#		endif//RBLXMATH_ARCH & RBLXMATH_ARCH_SSE_BIT
		return Result;
	}
}//namespace rblxmath

int test_vec4_load()
{
	int Error = 0;

	float Data[] = {1.f, 2.f, 3.f, 4.f};
	rblxmath::aligned_vec4 const V = rblxmath::loadu(Data);
	Error += rblxmath::all(rblxmath::equal(V, rblxmath::aligned_vec4(1.f, 2.f, 3.f, 4.f), rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}
#endif
*/
int main()
{
	int Error = 0;
/*
#	if RBLXMATH_CONFIG_SIMD == RBLXMATH_ENABLE
		Error += test_vec4_load();
#	endif
*/
	return Error;
}
