#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/matrix_factorisation.hpp>
#include <rblxmath/gtc/constants.hpp>
#include <rblxmath/gtc/epsilon.hpp>

template <rblxmath::length_t C, rblxmath::length_t R, typename T, rblxmath::qualifier Q>
static int test_qr(rblxmath::mat<C, R, T, Q> m)
{
	int Error = 0;

	T const epsilon = static_cast<T>(1e-10);

	rblxmath::mat<(C < R ? C : R), R, T, Q> q(-999);
	rblxmath::mat<C, (C < R ? C : R), T, Q> r(-999);

	rblxmath::qr_decompose(m, q, r);

	//Test if q*r really equals the input matrix
	rblxmath::mat<C, R, T, Q> tm = q*r;
	rblxmath::mat<C, R, T, Q> err = tm - m;

	for (rblxmath::length_t i = 0; i < C; i++)
	for (rblxmath::length_t j = 0; j < R; j++)
		Error += rblxmath::abs(err[i][j]) > epsilon ? 1 : 0;

	//Test if the columns of q are orthonormal
	for (rblxmath::length_t i = 0; i < (C < R ? C : R); i++)
	{
		Error += (length(q[i]) - 1) > epsilon ? 1 : 0;

		for (rblxmath::length_t j = 0; j<i; j++)
			Error += rblxmath::abs(dot(q[i], q[j])) > epsilon ? 1 : 0;
	}

	//Test if the matrix r is upper triangular
	for (rblxmath::length_t i = 0; i < C; i++)
	for (rblxmath::length_t j = i + 1; j < (C < R ? C : R); j++)
		Error += rblxmath::epsilonEqual(r[i][j], static_cast<T>(0), rblxmath::epsilon<T>()) ? 0 : 1;

	return Error;
}

template <rblxmath::length_t C, rblxmath::length_t R, typename T, rblxmath::qualifier Q>
static int test_rq(rblxmath::mat<C, R, T, Q> m)
{
	int Error = 0;

	T const epsilon = static_cast<T>(1e-10);

	rblxmath::mat<C, (C < R ? C : R), T, Q> q(-999);
	rblxmath::mat<(C < R ? C : R), R, T, Q> r(-999);

	rblxmath::rq_decompose(m, r, q);

	//Test if q*r really equals the input matrix
	rblxmath::mat<C, R, T, Q> tm = r*q;
	rblxmath::mat<C, R, T, Q> err = tm - m;

	for (rblxmath::length_t i = 0; i < C; i++)
	for (rblxmath::length_t j = 0; j < R; j++)
		Error += rblxmath::abs(err[i][j]) > epsilon ? 1 : 0;

	//Test if the rows of q are orthonormal
	rblxmath::mat<(C < R ? C : R), C, T, Q> tq = transpose(q);

	for (rblxmath::length_t i = 0; i < (C < R ? C : R); i++)
	{
		Error += (length(tq[i]) - 1) > epsilon ? 1 : 0;

		for (rblxmath::length_t j = 0; j<i; j++)
			Error += rblxmath::abs(dot(tq[i], tq[j])) > epsilon ? 1 : 0;
	}

	//Test if the matrix r is upper triangular
	for (rblxmath::length_t i = 0; i < (C < R ? C : R); i++)
	for (rblxmath::length_t j = R - (C < R ? C : R) + i + 1; j < R; j++)
		Error += rblxmath::epsilonEqual(r[i][j], static_cast<T>(0), rblxmath::epsilon<T>()) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	//Test QR square
	Error += test_qr(rblxmath::dmat3(12.0, 6.0, -4.0, -51.0, 167.0, 24.0, 4.0, -68.0, -41.0)) ? 1 : 0;

	//Test RQ square
	Error += test_rq(rblxmath::dmat3(12.0, 6.0, -4.0, -51.0, 167.0, 24.0, 4.0, -68.0, -41.0)) ? 1 : 0;

	//Test QR triangular 1
	Error += test_qr(rblxmath::dmat3x4(12.0, 6.0, -4.0, -51.0, 167.0, 24.0, 4.0, -68.0, -41.0, 7.0, 2.0, 15.0)) ? 1 : 0;

	//Test QR triangular 2
	Error += test_qr(rblxmath::dmat4x3(12.0, 6.0, -4.0, -51.0, 167.0, 24.0, 4.0, -68.0, -41.0, 7.0, 2.0, 15.0)) ? 1 : 0;

	//Test RQ triangular 1 : Fails at the triangular test
	Error += test_rq(rblxmath::dmat3x4(12.0, 6.0, -4.0, -51.0, 167.0, 24.0, 4.0, -68.0, -41.0, 7.0, 2.0, 15.0)) ? 1 : 0;

	//Test QR triangular 2
	Error += test_rq(rblxmath::dmat4x3(12.0, 6.0, -4.0, -51.0, 167.0, 24.0, 4.0, -68.0, -41.0, 7.0, 2.0, 15.0)) ? 1 : 0;

	return Error;
}
