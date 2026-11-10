#pragma once

#include <rblxgl/rblxgl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <rblxmath/rblxmath.hpp>

class Shader {
public:
    unsigned int ID;

    // False when the driver refused to build the program. Nothing drawn with an
    // unusable program appears at all, and the driver's complaint used to reach
    // only the log the game view hides, so it is kept for the caller to report.
    bool usable = false;
    std::string buildLog;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const rblxmath::vec3 &value) const;
    void setMat4(const std::string &name, const rblxmath::mat4 &mat) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};



