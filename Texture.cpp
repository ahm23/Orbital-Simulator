#include "Texture.h"
#include <GL/glew.h>
#include "stb_image.h"
#include "utils_debug.h"

Texture::Texture(const std::string& path) : id(0), filepath(path), localBuffer(nullptr), width(0), height(0), BPP(0)
{
	// Flip texture because OpenGL uses pixels from bottom left.
	//stbi_set_flip_vertically_on_load(1);
	localBuffer = stbi_load(path.c_str(), &width, &height, &BPP, 4);

	GL(glGenTextures(1, &id));
	GL(glBindTexture(GL_TEXTURE_2D, id));
	
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)); // horizontal
	GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); // vertical

	GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer));
		// GL_RGBA8 is how it stores texture
	GL(glBindTexture(GL_TEXTURE_2D, 0));

	if (localBuffer)
		stbi_image_free(localBuffer);
}

Texture::~Texture()
{
	GL(glDeleteTextures(1, &id));
}

void Texture::Bind(unsigned int slot) const
{
	GL(glActiveTexture(GL_TEXTURE0 + slot));
	GL(glBindTexture(GL_TEXTURE_2D, id));
}

void Texture::Unbind() const
{
	GL(glBindTexture(GL_TEXTURE_2D, 0));
}
