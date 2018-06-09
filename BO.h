#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

namespace gl
{
	class BO
	{
	public:
		static void Unbind(GLenum target);

		enum TYPES
		{
			VERTEXBUFFEROBJECT = GL_ARRAY_BUFFER,
			INDEXBUFFEROBJECT = GL_ELEMENT_ARRAY_BUFFER,
		};

		BO(GLenum target);
		~BO();

		void Bind();
		void Unbind();

		//VBO: Bind the VBO to a VAO
		//IBO: Bind the IBO to the VAO
		void SetBufferData(GLsizeiptr size, const GLvoid *data, GLenum usage);

		GLuint GetID();

	private:
		GLuint m_id;
		GLenum m_target;
	};
}