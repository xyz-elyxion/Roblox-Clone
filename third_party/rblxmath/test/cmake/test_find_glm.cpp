#include <iostream>
#include <rblxmath/rblxmath.hpp>
#include <rblxmath/ext.hpp>

rblxmath::mat4 camera(float Translate, rblxmath::vec2 const& Rotate)
{
	rblxmath::mat4 Projection = rblxmath::perspective(rblxmath::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
	rblxmath::mat4 View = rblxmath::translate(rblxmath::mat4(1.0f), rblxmath::vec3(0.0f, 0.0f, -Translate));
	View = rblxmath::rotate(View, Rotate.y, rblxmath::vec3(-1.0f, 0.0f, 0.0f));
	View = rblxmath::rotate(View, Rotate.x, rblxmath::vec3(0.0f, 1.0f, 0.0f));
	rblxmath::mat4 Model = rblxmath::scale(rblxmath::mat4(1.0f), rblxmath::vec3(0.5f));
	return Projection * View * Model;
}

int main()
{
    const rblxmath::mat4 m = camera(1.f, rblxmath::vec2(1.f, 0.5f));
    std::cout << "matrix diagonal: " << m[0][0] << ", "
              << m[1][1] << ", " << m[2][2] << ", " << m[3][3] << "\n";
    return 0;
}

