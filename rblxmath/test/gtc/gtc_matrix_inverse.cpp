#include <rblxmath/gtc/matrix_inverse.hpp>
#include <rblxmath/gtc/epsilon.hpp>

static int test_affine()
{
	int Error = 0;

	{
		rblxmath::mat3 const M(
			2.f, 0.f, 0.f,
			0.f, 2.f, 0.f,
			0.f, 0.f, 1.f);
		rblxmath::mat3 const A = rblxmath::affineInverse(M);
		rblxmath::mat3 const I = rblxmath::inverse(M);
		rblxmath::mat3 const R = rblxmath::affineInverse(A);

		for(rblxmath::length_t i = 0; i < A.length(); ++i)
		{
			Error += rblxmath::all(rblxmath::epsilonEqual(M[i], R[i], 0.01f)) ? 0 : 1;
			Error += rblxmath::all(rblxmath::epsilonEqual(A[i], I[i], 0.01f)) ? 0 : 1;
		}
	}

	{
		rblxmath::mat4 const M(
			2.f, 0.f, 0.f, 0.f,
			0.f, 2.f, 0.f, 0.f,
			0.f, 0.f, 2.f, 0.f,
			0.f, 0.f, 0.f, 1.f);
		rblxmath::mat4 const A = rblxmath::affineInverse(M);
		rblxmath::mat4 const I = rblxmath::inverse(M);
		rblxmath::mat4 const R = rblxmath::affineInverse(A);

		for(rblxmath::length_t i = 0; i < A.length(); ++i)
		{
			Error += rblxmath::all(rblxmath::epsilonEqual(M[i], R[i], 0.01f)) ? 0 : 1;
			Error += rblxmath::all(rblxmath::epsilonEqual(A[i], I[i], 0.01f)) ? 0 : 1;
		}
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_affine();

	return Error;
}
