#include "VAO.h"

namespace gl
{
	void VAO::Unbind()
	{
		glBindVertexArray(0);
	}

	VAO::VAO(unsigned int number_of_vertex_attribute_locations)
		: m_arrays(0), m_id(0)
	{
		glGenVertexArrays(1, &m_id);
		//gl::ExitOnGLError("ERROR: Could not generate the VAO");

		Bind();
		EnableGenericArraysAttribs(number_of_vertex_attribute_locations);
		Unbind();
	}

	VAO::~VAO()
	{
		glDeleteVertexArrays(1, &m_id);
		//gl::ExitOnGLError("ERROR: Could not destroy the vertex array");
	}

	void VAO::Bind()
	{
		GLint currentvaoid;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentvaoid);
		if (currentvaoid != m_id)
		{
			glBindVertexArray(m_id);
			//gl::ExitOnGLError("ERROR: Could not bind the VAO");
		}
	}

	void VAO::DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
	{
		Bind();
		glDrawElements(mode, count, type, indices);
		//gl::ExitOnGLError("lqc: error in GLVAO::DrawElements(...)");
	}

	void VAO::SetVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
	{
		if (index < m_arrays)
			glVertexAttribPointer(index, size, type, normalized, stride, pointer);
		//gl::ExitOnGLError("ERROR: Could not set VAO attributes");

	}

	void VAO::EnableGenericArraysAttribs(unsigned int number_of_arrays)
	{
		for (m_arrays = 0; m_arrays < number_of_arrays && m_arrays < GL_MAX_VERTEX_ATTRIBS; m_arrays++)
			glEnableVertexAttribArray(m_arrays);
		//gl::ExitOnGLError("ERROR: Could not enable vertex attributes");
	}

	void VAO::DisableGenericArraysAttribs()
	{
		for (int i = 0; i < (int)m_arrays; i++)
			glDisableVertexAttribArray(i);
	}

	GLuint VAO::GetID()
	{
		return m_id;
	}
}