#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

namespace gl
{
	class Texture1D
	{
	public:
		Texture1D(unsigned int length);
		~Texture1D();

		void GenerateTexture(GLint min_filter_param, GLint max_filter_param
			, GLint wrap_s_param);

		bool SetData(GLvoid* data, GLint internalformat, GLenum format, GLenum type);

		GLuint GetTextureID();

		unsigned int GetLength();
	private:
		void DestroyTexture();
		unsigned int m_size;
		unsigned int m_length;
		GLuint m_textureID;
	};
}

