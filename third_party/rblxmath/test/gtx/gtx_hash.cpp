#if __cplusplus >= 201103L

#include <rblxmath/detail/setup.hpp>

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wfloat-equal"
#endif

#define RBLXMATH_ENABLE_EXPERIMENTAL
#include <rblxmath/gtx/hash.hpp>

#include <unordered_map>

static int test_compile()
{
    int Error = 0;

    // Vector types
    std::unordered_map<rblxmath::ivec1, int> map_vec1;
    Error += ++map_vec1[rblxmath::ivec1(0)];
    std::unordered_map<rblxmath::ivec2, int> map_vec2;
    Error += ++map_vec2[rblxmath::ivec2(0)];
    std::unordered_map<rblxmath::ivec3, int> map_vec3;
    Error += ++map_vec3[rblxmath::ivec3(0)];
    std::unordered_map<rblxmath::ivec4, int> map_vec4;
    Error += ++map_vec4[rblxmath::ivec4(0)];

    // Quaternion types
    std::unordered_map<rblxmath::quat, int> map_quat;
    Error += ++map_quat[rblxmath::quat(0.0f, rblxmath::vec3(0.0f))];
    std::unordered_map<rblxmath::dualquat, int> map_dualquat;
    Error += ++map_dualquat[rblxmath::dualquat(rblxmath::vec3(0.0f))];

    // Matrix types
    std::unordered_map<rblxmath::mat2x2, int> map_mat2x2;
    Error += ++map_mat2x2[rblxmath::mat2x2(0.0f)];
    std::unordered_map<rblxmath::mat2x3, int> map_mat2x3;
    Error += ++map_mat2x3[rblxmath::mat2x3(0.0f)];
    std::unordered_map<rblxmath::mat2x4, int> map_mat2x4;
    Error += ++map_mat2x4[rblxmath::mat2x4(0.0f)];
    std::unordered_map<rblxmath::mat3x2, int> map_mat3x2;
    Error += ++map_mat3x2[rblxmath::mat3x2(0.0f)];
    std::unordered_map<rblxmath::mat3x3, int> map_mat3x3;
    Error += ++map_mat3x3[rblxmath::mat3x3(0.0f)];
    std::unordered_map<rblxmath::mat3x4, int> map_mat3x4;
    Error += ++map_mat3x4[rblxmath::mat3x4(0.0f)];
    std::unordered_map<rblxmath::mat4x2, int> map_mat4x2;
    Error += ++map_mat4x2[rblxmath::mat4x2(0.0f)];
    std::unordered_map<rblxmath::mat4x3, int> map_mat4x3;
    Error += ++map_mat4x3[rblxmath::mat4x3(0.0f)];
    std::unordered_map<rblxmath::mat4x4, int> map_mat4x4;
    Error += ++map_mat4x4[rblxmath::mat4x4(0.0f)];

    return Error > 0 ? 0 : 1;
}

int main()
{
    int Error = 0;

    Error += test_compile();

    return Error;
}

#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	pragma clang diagnostic pop
#endif

#else

int main()
{
    return 0;
}

#endif// __cplusplus >= 201103L


