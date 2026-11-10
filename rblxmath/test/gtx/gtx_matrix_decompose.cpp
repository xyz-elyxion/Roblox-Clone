#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/matrix_decompose.hpp>

#include <rblxmath/ext/matrix_relational.hpp>
#include <rblxmath/ext/matrix_transform.hpp>
#include <rblxmath/ext/scalar_constants.hpp>

static int test_identity() {
	int Error = 0;

	rblxmath::mat4 Matrix(1);

	rblxmath::vec3 Scale;
	rblxmath::quat Orientation;
	rblxmath::vec3 Translation;
	rblxmath::vec3 Skew(1);
	rblxmath::vec4 Perspective(1);

	rblxmath::decompose(Matrix, Scale, Orientation, Translation, Skew, Perspective);

	rblxmath::mat4 Out = rblxmath::recompose(Scale, Orientation, Translation, Skew, Perspective);

	Error += rblxmath::all(rblxmath::equal(Matrix, Out, rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

static int test_scale_translate() {
	int Error = 0;

	rblxmath::vec3 const T(2.0f);
	rblxmath::vec3 const S(2.0f);

	rblxmath::mat4 Matrix = rblxmath::translate(rblxmath::scale(rblxmath::mat4(1), S), T);

	rblxmath::vec3 Scale(2);
	rblxmath::quat Orientation;
	rblxmath::vec3 Translation(2);
	rblxmath::vec3 Skew(1);
	rblxmath::vec4 Perspective(1);

	rblxmath::decompose(Matrix, Scale, Orientation, Translation, Skew, Perspective);

	rblxmath::mat4 Out = rblxmath::recompose(Scale, Orientation, Translation, Skew, Perspective);

	Error += rblxmath::all(rblxmath::equal(Matrix, Out, rblxmath::epsilon<float>())) ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	Error += test_identity();
	Error += test_scale_translate();

	return Error;
}
