#define RBLXMATH_FORCE_SIZE_T_LENGTH
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/ext.hpp>

template <typename genType>
genType add(genType const& a, genType const& b)
{
	genType result(0);
	for(rblxmath::length_t i = 0; i < a.length(); ++i)
		result[i] = a[i] + b[i];
	return result;
}

int main()
{
	int Error = 0;

	rblxmath::ivec4 v(1);
	Error += add(v, v) == rblxmath::ivec4(2) ? 0 : 1;

	return Error;
}
