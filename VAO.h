#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

namespace gl
{
	class VAO
	{
	public:
		static void Unbind();

		VAO(unsigned int number_of_vertex_attribute_locations = 0);
		~VAO();

		void Bind();

		void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices = (GLvoid*)0);

		void SetVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);

		void EnableGenericArraysAttribs(unsigned int number_of_arrays);
		void DisableGenericArraysAttribs();

		GLuint GetID();

	private:
		GLuint m_id;
		unsigned int m_arrays;
	};
}
