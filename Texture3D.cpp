#include "Texture3D.h"
#include <cassert>
#include <GL/glew.h>
#include <iostream>
namespace gl
{
	Texture3D::Texture3D(unsigned int width, unsigned int height, unsigned int depth
		, unsigned int subwidth, unsigned int subheight, unsigned int subdepth)
	{
		m_width = width;
		m_height = height;
		m_depth = depth;
		m_subwidth = subwidth;
		m_subheight = subheight;
		m_subdepth = subdepth;
		int maxtex3d;
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxtex3d);
		assert(m_width <= maxtex3d || m_height <= maxtex3d || m_depth <= maxtex3d);
		m_textureID = -1;
	}

	Texture3D::~Texture3D()
	{
		DestroyTexture();
	}

	void Texture3D::GenerateTexture(GLint min_filter_param, GLint max_filter_param
		, GLint wrap_s_param, GLint wrap_t_param, GLint wrap_r_param)
	{
		if (m_textureID != -1)
			DestroyTexture();
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_3D, m_textureID);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, min_filter_param);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, max_filter_param);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_s_param);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_t_param);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_r_param);
	}

	bool Texture3D::SetData(GLvoid* data, GLint internalformat, GLenum format, GLenum type)
	{
		if (m_textureID == -1)
			return false;

		// Bind texture
		glBindTexture(GL_TEXTURE_3D, m_textureID);

		// Set Data
		glTexImage3D(GL_TEXTURE_3D, 0, internalformat, m_width, m_height, m_depth, 0, format, type, data);
#if _DEBUG
		//printf("texture3d.cpp: Texture generated with id %d!\n", m_textureID);
#endif

		// Unbind texture
		glBindTexture(GL_TEXTURE_3D, 0);

		//assert(glGetError() == GL_NO_ERROR);

		return true;
	}

	bool Texture3D::SetStorage(GLenum internalformat)
	{
		glTextureStorage3D(m_textureID, 1, internalformat, m_width, m_height, m_depth);
		return true;
	}
	bool Texture3D::SetSubData(GLvoid* subdata, int xoffset, int yoffset, int zoffset, GLenum format, GLenum type)
	{
		glBindTexture(GL_TEXTURE_3D, m_textureID);
		glTextureSubImage3D(m_textureID, 0, xoffset, yoffset, zoffset, m_subwidth, m_subheight, m_subdepth
			, format, type, subdata);
		glBindTexture(GL_TEXTURE_3D, 0);
		return true;
	}

	GLuint Texture3D::GetTextureID()
	{
		return m_textureID;
	}

	unsigned int Texture3D::GetWidth()
	{
		return m_width;
	}

	unsigned int Texture3D::GetHeight()
	{
		return m_height;
	}

	unsigned int Texture3D::GetDepth()
	{
		return m_depth;
	}

	void Texture3D::DestroyTexture()
	{
		GLint temp_texture = m_textureID;
		glDeleteTextures(1, &m_textureID);
#if _DEBUG
		// printf("lqc: Texture3D with id %d destroyed!\n", temp_texture);
#endif
		m_textureID = -1;
	}
}