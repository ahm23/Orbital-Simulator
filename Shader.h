#pragma once
#include <iostream>
#include <unordered_map>
#include <glm.hpp>


class Shader
{
public:
	unsigned int id;
	std::string fn_vertex;
	std::string fn_fragment;
	std::unordered_map<std::string, int> locationCache;

	Shader(const std::string& filename_vertex, const std::string& filename_fragment);
	~Shader();

	void bind() const;
	void unbind() const;

	void SetUniform1i(const std::string& name, int v);
	void SetUniform1f(const std::string& name, float v);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const glm::mat4 &matrix);
private:
	std::string loadShader(const std::string &filepath);
	unsigned int CompileShader(unsigned int type, const std::string &source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	int GetUniformLocation(const std::string& name);
};

