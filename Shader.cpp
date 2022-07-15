#include "Shader.h"
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include "utils_debug.h"


Shader::Shader(const std::string& filename_vertex, const std::string& filename_fragment): fn_vertex(filename_vertex), fn_fragment(filename_fragment), id(0)
{
    id = CreateShader(loadShader(filename_vertex), loadShader(filename_fragment));
}

Shader::~Shader()
{
    GL(glDeleteProgram(id));
}

void Shader::bind() const
{
    GL(glUseProgram(id));
}

void Shader::unbind() const
{
    GL(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int v)
{
    GL(glUniform1i(GetUniformLocation(name), v));
}

void Shader::SetUniform1f(const std::string& name, float v)
{
    GL(glUniform1f(GetUniformLocation(name), v));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    GL(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name),v0,v1,v2,v3);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    GL(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

std::string Shader::loadShader(const std::string &filepath) {
    std::ifstream stream(filepath);
    std::string line;
    std::stringstream s;
    while (getline(stream, line)) {
        s << line << '\n';
    }

    return s.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    GL(glShaderSource(id, 1, &src, nullptr));
    GL(glCompileShader(id));

    int result;
    GL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (!result) {
        int len;
        GL(glGetShaderiv(id, GL_COMPILE_STATUS, &len));
        char* message = (char*)malloc(len * sizeof(char));
        GL(glGetShaderInfoLog(id, len, &len, message));
        std::cout << message << std::endl;
        GL(glDeleteShader(id));
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GL(glAttachShader(program, vs));
    GL(glAttachShader(program, fs));
    GL(glLinkProgram(program));
    GL(glValidateProgram(program));

    GL(glDeleteShader(vs));
    GL(glDeleteShader(fs));

    return program;
}


int Shader::GetUniformLocation(const std::string& name)
{
    if (locationCache.find(name) != locationCache.end())
        return locationCache[name];

    int location = glGetUniformLocation(id, name.c_str());
    if (location == -1) {
        std::cout << "Warning: uniform '" << name << "' doesn't exist." << std::endl;
        locationCache[name] = location;
    }
    return location;
}

