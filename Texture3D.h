#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

namespace gl
{
	class Texture3D
	{
	public:
		Texture3D(unsigned int width, unsigned int height, unsigned int depth
				, unsigned int subwidth, unsigned int subheight, unsigned int subdepth);
		~Texture3D();

		void GenerateTexture(GLint min_filter_param, GLint max_filter_param
			, GLint wrap_s_param, GLint wrap_t_param, GLint wrap_r_param);

		bool SetData(GLvoid* data, GLint internalformat, GLenum format, GLenum type);
		bool SetStorage(GLenum internalformat);
		bool SetSubData(GLvoid* subdata, int xoffset, int yoffset, int zoffset, GLenum format, GLenum type);

		GLuint GetTextureID();

		unsigned int GetWidth();
		unsigned int GetHeight();
		unsigned int GetDepth();

	protected:

	private:
		void DestroyTexture();
		unsigned int m_width;
		unsigned int m_height;
		unsigned int m_depth;
		unsigned int m_subwidth;
		unsigned int m_subheight;
		unsigned int m_subdepth;

		GLuint m_textureID;
	};
}

