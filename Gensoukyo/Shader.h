#pragma once
#include "pch.h"
class Shader
{
public:
	unsigned ID;

    // constructor reads and builds the shader
    Shader();
    Shader( const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void compile( const char* vertexPath, const char* fragmentPath);
    void use();
    void disable();

    // utility uniform functions
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
};

