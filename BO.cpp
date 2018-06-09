#include "BO.h"

namespace gl
{
	void BO::Unbind(GLenum target)
	{
		glBindBuffer(target, 0);
	}

	BO::BO(GLenum target)
		: m_id(0), m_target(target)
	{
		glGenBuffers(1, &m_id);
		//gl::ExitOnGLError("ERROR: Could not generate the Buffer Object");
	}

	BO::~BO()
	{
		glDeleteBuffers(1, &m_id);
		//gl::ExitOnGLError("ERROR: Could not destroy the buffer object");
	}

	void BO::Bind()
	{
		glBindBuffer(m_target, m_id);
		//gl::ExitOnGLError("ERROR: Could not bind the Buffer Object");
	}

	void BO::Unbind()
	{
		glBindBuffer(m_target, 0);
	}

	void BO::SetBufferData(GLsizeiptr size, const GLvoid *data, GLenum usage)
	{
		Bind();
		glBufferData(m_target, size, data, usage);
		//gl::ExitOnGLError("ERROR: Could not set Buffer Object data");
	}

	GLuint BO::GetID()
	{
		return m_id;
	}
}