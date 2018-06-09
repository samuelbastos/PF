#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

namespace gl
{
	class TextureArray
	{
	public:
		TextureArray(unsigned int width, unsigned int height, unsigned int layers);
		~TextureArray();

		void GenerateTexture(GLint min_filter_param, GLint max_filter_param
			, GLint wrap_s_param, GLint wrap_t_param);

		bool SetData(GLvoid* data, GLint internalformat, GLenum format, GLenum type);

		GLuint GetTextureID();

		unsigned int GetWidth();
		unsigned int GetHeight();

	protected:

	private:
		void DestroyTexture();
		unsigned int m_width;
		unsigned int m_height;
		unsigned int m_layers;
		GLuint m_textureID;
	};
}

