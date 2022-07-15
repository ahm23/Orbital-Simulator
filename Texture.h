#pragma once
#include <iostream>

class Texture
{
public:
	Texture(const std::string &path);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

private:
	unsigned int id;
	std::string filepath;
	unsigned char* localBuffer;
	int width, height, BPP;
};

