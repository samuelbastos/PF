#include "TextureArray.h"


#include <cassert>
#define GLEW_STATIC
#include <GL/glew.h>

namespace gl
{
	TextureArray::TextureArray(unsigned int width, unsigned int height, unsigned int layers)
	{
		m_width = width;
		m_height = height;
		m_layers = layers;

		int maxtexd;
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxtexd);
		assert(m_width <= maxtexd || m_height <= maxtexd);
		m_textureID = -1;
	}

	TextureArray::~TextureArray()
	{
		DestroyTexture();
	}

	void TextureArray::GenerateTexture(GLint min_filter_param, GLint max_filter_param
		, GLint wrap_s_param, GLint wrap_t_param)
	{
		if (m_textureID != -1)
			DestroyTexture();
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, min_filter_param);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, max_filter_param);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap_s_param);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap_t_param);
	}

	bool TextureArray::SetData(GLvoid* data, GLint internalformat, GLenum format, GLenum type)
	{
		if (m_textureID == -1)
			return false;

		// Bind texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);

		// Set Data
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalformat, m_width, m_height, m_layers, 0, format, type, data);
#if _DEBUG
		//printf("texture3d.cpp: Texture generated with id %d!\n", m_textureID);
#endif

		// Unbind texture
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		//assert(glGetError() == GL_NO_ERROR);

		return true;
	}

	GLuint TextureArray::GetTextureID()
	{
		return m_textureID;
	}

	unsigned int TextureArray::GetWidth()
	{
		return m_width;
	}

	unsigned int TextureArray::GetHeight()
	{
		return m_height;
	}

	void TextureArray::DestroyTexture()
	{
		GLint temp_texture = m_textureID;
		glDeleteTextures(1, &m_textureID);
#if _DEBUG
		// printf("lqc: Texture3D with id %d destroyed!\n", temp_texture);
#endif
		m_textureID = -1;
	}
}