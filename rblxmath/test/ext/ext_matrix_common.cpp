#include <rblxmath/ext/matrix_common.hpp>
#include <rblxmath/ext/matrix_double4x4.hpp>
#include <rblxmath/ext/matrix_float4x4.hpp>
#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/ext/vector_bool4.hpp>
#include <rblxmath/ext/matrix_float4x3.hpp>

static int test_mix()
{
	int Error = 0;

	{
		rblxmath::mat4 A(2);
		rblxmath::mat4 B(4);
		rblxmath::mat4 C = rblxmath::mix(A, B, 0.5f);
		rblxmath::bvec4 const D = rblxmath::equal(C, rblxmath::mat4(3), 1);
		Error += rblxmath::all(D) ? 0 : 1;
	}

	{
		rblxmath::mat4 A(2);
		rblxmath::mat4 B(4);
		rblxmath::mat4 C = rblxmath::mix(A, B, 0.5);
		rblxmath::bvec4 const D = rblxmath::equal(C, rblxmath::mat4(3), 1);
		Error += rblxmath::all(D) ? 0 : 1;
	}

	{
		rblxmath::dmat4 A(2);
		rblxmath::dmat4 B(4);
		rblxmath::dmat4 C = rblxmath::mix(A, B, 0.5);
		rblxmath::bvec4 const D = rblxmath::equal(C, rblxmath::dmat4(3), 1);
		Error += rblxmath::all(D) ? 0 : 1;
	}

	{
		rblxmath::dmat4 A(2);
		rblxmath::dmat4 B(4);
		rblxmath::dmat4 C = rblxmath::mix(A, B, 0.5f);
		rblxmath::bvec4 const D = rblxmath::equal(C, rblxmath::dmat4(3), 1);
		Error += rblxmath::all(D) ? 0 : 1;
	}

	return Error;
}

static int test_abs()
{
	int Error = 0;

	// -------------------- //
	// rblxmath::mat4 variants : //
	// -------------------- //
	{
		rblxmath::mat4 A(
			3.0f, 1.0f, 5.2f, 4.9f,
			1.4f, 0.5f, 9.3f, 3.7f,
			6.8f, 8.4f, 4.3f, 3.9f,
			5.6f, 7.2f, 1.1f, 4.4f
		);
		rblxmath::mat4 B(
			 1.0,-1.0, 1.0, 1.0,
			-1.0, 1.0, 1.0,-1.0,
			 1.0,-1.0,-1.0,-1.0,
			-1.0,-1.0, 1.0, 1.0
		);
		rblxmath::mat4 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat4 D = rblxmath::abs(C);
		rblxmath::bvec4 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec4 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		rblxmath::bvec4 const col3 = rblxmath::equal(D[2], A[2], 0.001f);
		rblxmath::bvec4 const col4 = rblxmath::equal(D[3], A[3], 0.001f);
		Error += rblxmath::all(rblxmath::bvec4(rblxmath::all(col1), rblxmath::all(col2), rblxmath::all(col3), rblxmath::all(col4))) ? 0 : 1;
	}
	{
		rblxmath::mat4x3 A(
			3.0f, 1.0f, 5.2f,
			4.9f, 1.4f, 0.5f,
			9.3f, 3.7f, 6.8f,
			8.4f, 4.3f, 3.9f
		);
		rblxmath::mat4x3 B(
			 1.0,-1.0, 1.0,
			 1.0,-1.0, 1.0,
			 1.0,-1.0, 1.0,
			-1.0,-1.0,-1.0
		);
		rblxmath::mat4x3 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat4x3 D = rblxmath::abs(C);
		rblxmath::bvec3 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec3 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		rblxmath::bvec3 const col3 = rblxmath::equal(D[2], A[2], 0.001f);
		rblxmath::bvec3 const col4 = rblxmath::equal(D[3], A[3], 0.001f);
		Error += rblxmath::all(rblxmath::bvec4(rblxmath::all(col1), rblxmath::all(col2), rblxmath::all(col3), rblxmath::all(col4))) ? 0 : 1;
	}
	{
		rblxmath::mat4x2 A(
			3.0f, 1.0f,
			1.4f, 0.5f,
			6.8f, 8.4f,
			5.6f, 7.2f
		);
		rblxmath::mat4x2 B(
			 1.0,-1.0,
			-1.0, 1.0,
			 1.0,-1.0,
			-1.0,-1.0
		);
		rblxmath::mat4x2 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat4x2 D = rblxmath::abs(C);
		rblxmath::bvec2 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec2 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		rblxmath::bvec2 const col3 = rblxmath::equal(D[2], A[2], 0.001f);
		rblxmath::bvec2 const col4 = rblxmath::equal(D[3], A[3], 0.001f);
		Error += rblxmath::all(rblxmath::bvec4(rblxmath::all(col1), rblxmath::all(col2), rblxmath::all(col3), rblxmath::all(col4))) ? 0 : 1;
	}

	// -------------------- //
	// rblxmath::mat3 variants : //
	// -------------------- //
	{
		rblxmath::mat3x4 A(
			3.0f, 1.0f, 5.2f, 4.9f,
			1.4f, 0.5f, 9.3f, 3.7f,
			6.8f, 8.4f, 4.3f, 3.9f
		);
		rblxmath::mat3x4 B(
			 1.0,-1.0, 1.0, 1.0,
			-1.0, 1.0, 1.0,-1.0,
			 1.0,-1.0,-1.0,-1.0
		);
		rblxmath::mat3x4 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat3x4 D = rblxmath::abs(C);
		rblxmath::bvec4 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec4 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		rblxmath::bvec4 const col3 = rblxmath::equal(D[2], A[2], 0.001f);
		Error += rblxmath::all(rblxmath::bvec3(rblxmath::all(col1), rblxmath::all(col2), rblxmath::all(col3))) ? 0 : 1;
	}
	{
		rblxmath::mat3 A(
			3.0f, 1.0f, 5.2f,
			1.4f, 0.5f, 9.3f,
			6.8f, 8.4f, 4.3f
		);
		rblxmath::mat3 B(
			1.0,-1.0, 1.0,
			-1.0, 1.0, 1.0,
			1.0,-1.0,-1.0
		);
		rblxmath::mat3 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat3 D = rblxmath::abs(C);
		rblxmath::bvec3 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec3 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		rblxmath::bvec3 const col3 = rblxmath::equal(D[2], A[2], 0.001f);
		Error += rblxmath::all(rblxmath::bvec3(rblxmath::all(col1), rblxmath::all(col2), rblxmath::all(col3))) ? 0 : 1;
	}
	{
		rblxmath::mat3x2 A(
			5.2f, 4.9f,
			9.3f, 3.7f,
			4.3f, 3.9f
		);
		rblxmath::mat3x2 B(
			 1.0, 1.0,
			 1.0,-1.0,
			-1.0,-1.0
		);
		rblxmath::mat3x2 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat3x2 D = rblxmath::abs(C);
		rblxmath::bvec2 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec2 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		rblxmath::bvec2 const col3 = rblxmath::equal(D[2], A[2], 0.001f);
		Error += rblxmath::all(rblxmath::bvec3(rblxmath::all(col1), rblxmath::all(col2), rblxmath::all(col3))) ? 0 : 1;
	}

	// -------------------- //
	// rblxmath::mat2 variants : //
	// -------------------- //
	{
		rblxmath::mat2x4 A(
			3.0f, 1.0f, 5.2f, 4.9f,
			5.6f, 7.2f, 1.1f, 4.4f
		);
		rblxmath::mat2x4 B(
			 1.0,-1.0, 1.0, 1.0,
			-1.0,-1.0, 1.0, 1.0
		);
		rblxmath::mat2x4 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat2x4 D = rblxmath::abs(C);
		rblxmath::bvec4 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec4 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		Error += rblxmath::all(rblxmath::bvec2(rblxmath::all(col1), rblxmath::all(col2))) ? 0 : 1;
	}
	{
		rblxmath::mat2x3 A(
			3.0f, 1.0f, 5.2f,
			8.4f, 4.3f, 3.9f
		);
		rblxmath::mat2x3 B(
			 1.0,-1.0, 1.0,
			-1.0,-1.0,-1.0
		);
		rblxmath::mat2x3 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat2x3 D = rblxmath::abs(C);
		rblxmath::bvec3 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec3 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		Error += rblxmath::all(rblxmath::bvec2(rblxmath::all(col1), rblxmath::all(col2))) ? 0 : 1;
	}
	{
		rblxmath::mat2 A(
			3.0f, 1.0f,
			5.6f, 7.2f
		);
		rblxmath::mat2 B(
			 1.0,-1.0,
			-1.0,-1.0
		);
		rblxmath::mat2 C = rblxmath::matrixCompMult(A, B); // Not * to avoid matrix product.
		rblxmath::mat2 D = rblxmath::abs(C);
		rblxmath::bvec2 const col1 = rblxmath::equal(D[0], A[0], 0.001f);
		rblxmath::bvec2 const col2 = rblxmath::equal(D[1], A[1], 0.001f);
		Error += rblxmath::all(rblxmath::bvec2(rblxmath::all(col1), rblxmath::all(col2))) ? 0 : 1;
	}

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_mix();
	Error += test_abs();

	return Error;
}
