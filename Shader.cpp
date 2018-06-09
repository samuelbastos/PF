#include "Shader.h"

#include <GL/glew.h>

#include <cstdio>
#include <iostream>
#include <cstring>
#include <sstream>

namespace gl
{
	Shader::UniformVariable::UniformVariable()
	{
		value = NULL;
		type = GLSL_VAR_TYPES::NONE;
		m_loc = -1;
	}

	Shader::UniformVariable::~UniformVariable()
	{}

	void Shader::UniformVariable::Bind()
	{
		float* aux;
		switch (type)
		{
		case GLSL_VAR_TYPES::UINT:
			glUniform1i(m_loc, *(unsigned int*)value);
			break;
		case GLSL_VAR_TYPES::INT:
			glUniform1i(m_loc, *(int*)value);
			break;
		case GLSL_VAR_TYPES::FLOAT:
			glUniform1f(m_loc, *(float*)value);
			break;
		case GLSL_VAR_TYPES::DOUBLE:
			glUniform1d(m_loc, *(double*)value);
			break;
		case GLSL_VAR_TYPES::GLMVEC2:
		case GLSL_VAR_TYPES::VECTOR2:
			aux = (float*)value;
			glUniform2f(m_loc, aux[0], aux[1]);
			break;
		case GLSL_VAR_TYPES::GLMVEC3:
		case GLSL_VAR_TYPES::VECTOR3:
		case GLSL_VAR_TYPES::FLOAT3:
			aux = (float*)value;
			glUniform3f(m_loc, aux[0], aux[1], aux[2]);
			break;
		case GLSL_VAR_TYPES::GLMVEC4:
		case GLSL_VAR_TYPES::VECTOR4:
		case GLSL_VAR_TYPES::FLOAT4:
			aux = (float*)value;
			glUniform4f(m_loc, aux[0], aux[1], aux[2], aux[3]);
			break;
		case GLSL_VAR_TYPES::MATRIX2:
			glUniformMatrix2fv(m_loc, 1, GL_FALSE, (float*)value);
			break;
		case GLSL_VAR_TYPES::MATRIX3:
		case GLSL_VAR_TYPES::FLOAT3X3:
			glUniformMatrix3fv(m_loc, 1, GL_FALSE, (float*)value);
			break;
		case GLSL_VAR_TYPES::GLMMAT4:
		case GLSL_VAR_TYPES::MATRIX4:
		case GLSL_VAR_TYPES::FLOAT4X4:
			glUniformMatrix4fv(m_loc, 1, GL_FALSE, (float*)value);
			break;
		case GLSL_VAR_TYPES::TEXTURE1D:
		{
			GLuint *aux = (GLuint*)value;
			glActiveTexture(GL_TEXTURE0 + aux[1]);
			glBindTexture(GL_TEXTURE_1D, aux[0]);
			glUniform1i(m_loc, aux[1]);
		}
		break;
		case GLSL_VAR_TYPES::TEXTURE2D:
		{
			GLuint *aux = (GLuint*)value;
			glActiveTexture(GL_TEXTURE0 + aux[1]);
			glBindTexture(GL_TEXTURE_2D, aux[0]);
			glUniform1i(m_loc, aux[1]);
		}
		break;
		case GLSL_VAR_TYPES::TEXTURE3D:
		{
			GLuint *aux = (GLuint*)value;
			glActiveTexture(GL_TEXTURE0 + aux[1]);
			glBindTexture(GL_TEXTURE_3D, aux[0]);
			glUniform1i(m_loc, aux[1]);
		}
		break;
		case GLSL_VAR_TYPES::TEXTURERECTANGLE:
		{
			GLuint *aux = (GLuint*)value;
			glActiveTexture(GL_TEXTURE0 + aux[1]);
			glBindTexture(GL_TEXTURE_RECTANGLE, aux[0]);
			glUniform1i(m_loc, aux[1]);
			glActiveTexture(GL_TEXTURE0);
		}
		case GLSL_VAR_TYPES::TEXTUREARRAY:
		{
			GLuint *aux = (GLuint*)value;
			glActiveTexture(GL_TEXTURE0 + aux[1]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, aux[0]);
			glUniform1i(m_loc, aux[1]);
		}
		break;
		};
	}

	void Shader::UniformVariable::DestroyValue()
	{
		if (value)
		{
			switch (type)
			{
			case GLSL_VAR_TYPES::UINT:
			{
				unsigned int *v = (unsigned int*)value;
				delete v;
			}
			break;
			case GLSL_VAR_TYPES::INT:
			{
				int *v = (int*)value;
				delete v;
			}
			break;
			case GLSL_VAR_TYPES::FLOAT:
			{
				float *v = (float*)value;
				delete v;
			}
			break;
			case GLSL_VAR_TYPES::DOUBLE:
			{
				double *v = (double*)value;
				delete v;
			}
			break;
			case GLSL_VAR_TYPES::GLMVEC2:
			case GLSL_VAR_TYPES::GLMVEC3:
			case GLSL_VAR_TYPES::GLMVEC4:
			case GLSL_VAR_TYPES::GLMMAT4:
			case GLSL_VAR_TYPES::VECTOR2:
			case GLSL_VAR_TYPES::VECTOR3:
			case GLSL_VAR_TYPES::FLOAT3:
			case GLSL_VAR_TYPES::VECTOR4:
			case GLSL_VAR_TYPES::FLOAT4:
			case GLSL_VAR_TYPES::MATRIX2:
			case GLSL_VAR_TYPES::MATRIX3:
			case GLSL_VAR_TYPES::FLOAT3X3:
			case GLSL_VAR_TYPES::MATRIX4:
			case GLSL_VAR_TYPES::FLOAT4X4: {
				float *v = (float*)value;
				delete[] v;
			}
										   break;
			case TEXTURE1D:
			case TEXTURE2D:
			case TEXTURE3D:
			case GLSL_VAR_TYPES::TEXTURERECTANGLE:
			{
				GLuint *v = (GLuint*)value;
				delete[] v;
			}
			break;
			};
		}
	}

	void Shader::Unbind()
	{
		glUseProgram(0);
	}

	char* Shader::TextFileRead(const char* file_name)
	{
#ifdef _DEBUG
		printf("File Name TextFileRead \"%s\"\n", file_name);
#endif
		FILE *file_source;
		char *content = NULL;
		int count = 0;
		if (file_name != NULL)
		{
			file_source = fopen(file_name, "rt");

			if (file_source != NULL)
			{
				fseek(file_source, 0, SEEK_END);
				count = ftell(file_source);
				rewind(file_source);

				if (count > 0) {
					content = (char *)malloc(sizeof(char)* (count + 1));
					count = (int)fread(content, sizeof(char), count, file_source);
					content[count] = '\0';
				}
				fclose(file_source);
			}
			else
			{
				printf("\nFile \"%s\" not found", file_name);
				getchar();
				exit(1);
			}
		}
		return content;
	}

	void Shader::CompileShader(GLuint shader_id, std::string filename)
	{
		char* shader_source = Shader::TextFileRead(filename.c_str());
		const char* const_shader_source = shader_source;

		glShaderSource(shader_id, 1, &const_shader_source, NULL);
		free(shader_source);

		glCompileShader(shader_id);
	}

	Shader::Shader()
	{
		m_shader_program = -1;

		vec_vertex_shaders_names.clear();
		vec_vertex_shaders_ids.clear();
		vec_fragment_shaders_names.clear();
		vec_fragment_shaders_ids.clear();
		vec_geometry_shaders_names.clear();
		vec_geometry_shaders_ids.clear();

	}

	bool Shader::AddShaderFile(SHADER_TYPE type, std::string filename)
	{
		if (type == SHADER_TYPE::VERTEX)
			vec_vertex_shaders_names.push_back(filename);
		else if (type == SHADER_TYPE::FRAGMENT)
			vec_fragment_shaders_names.push_back(filename);
		else if (type == SHADER_TYPE::GEOMETRY)
			vec_geometry_shaders_names.push_back(filename);
		//gl::ExitOnGLError("GLShader: Unable to attach a new shader file.");
		return true;
	}

	bool Shader::LoadAndLink()
	{
		if (m_shader_program == -1)
			m_shader_program = glCreateProgram();

		for (unsigned int i = 0; i < vec_vertex_shaders_names.size(); i++)
		{
			GLuint new_shader = glCreateShader(GL_VERTEX_SHADER);
			CompileShader(new_shader, vec_vertex_shaders_names[i]);

			vec_vertex_shaders_ids.push_back(new_shader);
			assert(vec_vertex_shaders_ids[i] == new_shader);

			glAttachShader(m_shader_program, new_shader);
		}

		for (unsigned int i = 0; i < vec_fragment_shaders_names.size(); i++)
		{
			GLuint new_shader = glCreateShader(GL_FRAGMENT_SHADER);
			CompileShader(new_shader, vec_fragment_shaders_names[i]);

			vec_fragment_shaders_ids.push_back(new_shader);
			assert(vec_fragment_shaders_ids[i] == new_shader);

			glAttachShader(m_shader_program, new_shader);
		}

		for (unsigned int i = 0; i < vec_geometry_shaders_names.size(); i++)
		{
			GLuint new_shader = glCreateShader(GL_GEOMETRY_SHADER);
			CompileShader(new_shader, vec_geometry_shaders_names[i]);

			vec_geometry_shaders_ids.push_back(new_shader);
			assert(vec_geometry_shaders_ids[i] == new_shader);

			glAttachShader(m_shader_program, new_shader);
		}

		glLinkProgram(m_shader_program);

		//gl::ExitOnGLError("GLShader: Unable to load and link shaders.");
		return true;
	}

	Shader::Shader(std::string vert, std::string frag)
		: Shader()
	{
		m_hasGeometryShader = false;
		if (GLEW_ARB_vertex_shader)
		{
#ifdef _DEBUG
			printf("Ready for GLSL - vertex and fragment units\n");
#endif
		}
		else {
			printf("lqc: Error on GLShader\n");
			exit(1);
		}

		AddShaderFile(SHADER_TYPE::VERTEX, vert);
		AddShaderFile(SHADER_TYPE::FRAGMENT, frag);
		LoadAndLink();

		Unbind();

		m_uniform_variables.clear();
	}


	Shader::Shader(char *vert, char *frag, char* geom)
		: Shader()
	{
		if (geom != NULL)
		{
			m_hasGeometryShader = true;
			if (glewIsSupported("GL_VERSION_4_0") && GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
			{
#ifdef _DEBUG
				printf("Ready for GLSL - vertex, fragment, and geometry units\n");
#endif
			}
			else {
				printf("lqc: Error on GLShader\n");
				exit(1);
			}
		}
		else
		{
			m_hasGeometryShader = false;
			if (GLEW_ARB_vertex_shader)
			{
#ifdef _DEBUG
				printf("Ready for GLSL - vertex and fragment units\n");
#endif
			}
			else {
				printf("lqc: Error on GLShader\n");
				exit(1);
			}
		}

		AddShaderFile(SHADER_TYPE::VERTEX, vert);
		AddShaderFile(SHADER_TYPE::FRAGMENT, frag);
		if (m_hasGeometryShader) AddShaderFile(SHADER_TYPE::GEOMETRY, geom);
		LoadAndLink();

		Unbind();

		m_uniform_variables.clear();
	}

	Shader::~Shader()
	{
		Clear();
		ClearUniforms();
		//gl::ExitOnGLError("ERROR: Could not destroy the shaders");
	}

	void Shader::Bind()
	{
		glUseProgram(m_shader_program);
		//gl::ExitOnGLError("ERROR: Could not use the shader program");
	}

	void Shader::Clear(bool delete_program)
	{
		Unbind();

		for (unsigned int i = 0; i < vec_fragment_shaders_ids.size(); i++)
		{
			glDetachShader(m_shader_program, vec_fragment_shaders_ids[i]);
			glDeleteShader(vec_fragment_shaders_ids[i]);
		}

		for (unsigned int i = 0; i < vec_geometry_shaders_ids.size(); i++)
		{
			glDetachShader(m_shader_program, vec_geometry_shaders_ids[i]);
			glDeleteShader(vec_geometry_shaders_ids[i]);
		}

		for (unsigned int i = 0; i < vec_vertex_shaders_ids.size(); i++)
		{
			glDetachShader(m_shader_program, vec_vertex_shaders_ids[i]);
			glDeleteShader(vec_vertex_shaders_ids[i]);
		}

		vec_vertex_shaders_ids.clear();
		vec_fragment_shaders_ids.clear();
		vec_geometry_shaders_ids.clear();

		// if glDeleteProgram is called before, we don't need to detach 
		//   the shaders anymore.
		if (delete_program)
		{
			glDeleteProgram(m_shader_program);
			m_shader_program = -1;
		}
	}

	void Shader::Reload()
	{
		Clear(false);

		LoadAndLink();
		Bind();

		for (std::map<std::string, Shader::UniformVariable>::iterator it = m_uniform_variables.begin(); it != m_uniform_variables.end(); ++it)
			m_uniform_variables[it->first].m_loc = glGetUniformLocation(m_shader_program, it->first.c_str());
		BindUniforms();

		printf("Shader reloaded with id = %d\n", m_shader_program);
	}

	GLint Shader::GetUniformLoc(char* name)
	{
		int uniform_location = glGetUniformLocation(m_shader_program, name);
		return uniform_location;
	}

	GLint Shader::GetAttribLoc(char* name)
	{
		int attrib_location = glGetAttribLocation(m_shader_program, name);
		return attrib_location;
	}

	GLuint Shader::GetProgramID()
	{
		return m_shader_program;
	}

	void Shader::BindUniforms()
	{
		for (std::map<std::string, Shader::UniformVariable>::iterator it = m_uniform_variables.begin(); it != m_uniform_variables.end(); ++it)
			m_uniform_variables[it->first].Bind();
		//gl::ExitOnGLError("ERROR: Could not set the shader uniforms");
	}

	void Shader::ClearUniforms()
	{
		for (std::map<std::string, Shader::UniformVariable>::iterator it = m_uniform_variables.begin(); it != m_uniform_variables.end(); ++it)
			it->second.DestroyValue();
		m_uniform_variables.clear();
	}

	void Shader::BindUniform(std::string var_name)
	{
		if (m_uniform_variables.find(var_name) != m_uniform_variables.end())
		{
			m_uniform_variables[var_name].Bind();
			//gl::ExitOnGLError("ERROR: Could not Bind Uniform");
		}
		else
		{
			std::cout << "lqc: Uniform " << var_name << " Not Found!" << std::endl;
		}
	}

	void Shader::SetUniform(std::string name, unsigned int value)
	{
		unsigned int* input_uniform = new unsigned int();
		*input_uniform = value;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::UINT;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniform(std::string name, int value)
	{
		int* input_uniform = new int();
		*input_uniform = value;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::INT;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniform(std::string name, float value)
	{
		float* input_uniform = new float();
		*input_uniform = value;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::FLOAT;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniform(std::string name, double value)
	{
		double* input_uniform = new double();
		*input_uniform = value;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::DOUBLE;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniform(std::string name, glm::vec2 value)
	{
		float* input_uniform = new float[2];
		input_uniform[0] = value.x;
		input_uniform[1] = value.y;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::GLMVEC2;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniform(std::string name, glm::vec3 value)
	{
		float* input_uniform = new float[3];
		input_uniform[0] = value.x;
		input_uniform[1] = value.y;
		input_uniform[2] = value.z;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::GLMVEC3;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniform(std::string name, glm::vec4 value)
	{
		float* input_uniform = new float[4];
		input_uniform[0] = value.x;
		input_uniform[1] = value.y;
		input_uniform[2] = value.z;
		input_uniform[3] = value.w;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::GLMVEC4;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniform(std::string name, glm::mat4 value)
	{
		float* input_uniform = new float[16];
		input_uniform[0] = value[0][0];
		input_uniform[1] = value[0][1];
		input_uniform[2] = value[0][2];
		input_uniform[3] = value[0][3];
		input_uniform[4] = value[1][0];
		input_uniform[5] = value[1][1];
		input_uniform[6] = value[1][2];
		input_uniform[7] = value[1][3];
		input_uniform[8] = value[2][0];
		input_uniform[9] = value[2][1];
		input_uniform[10] = value[2][2];
		input_uniform[11] = value[2][3];
		input_uniform[12] = value[3][0];
		input_uniform[13] = value[3][1];
		input_uniform[14] = value[3][2];
		input_uniform[15] = value[3][3];

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = GLSL_VAR_TYPES::GLMMAT4;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}

	void Shader::SetUniformTexture1D(std::string name, GLuint textureid, GLuint activeTextureid)
	{
		SetUniformTexture(name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTURE1D);
	}

	void Shader::SetUniformTexture2D(std::string name, GLuint textureid, GLuint activeTextureid)
	{
		SetUniformTexture(name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTURE2D);
	}

	void Shader::SetUniformTextureRectangle(std::string name, GLuint textureid, GLuint activeTextureid)
	{
		SetUniformTexture(name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTURERECTANGLE);
	}

	void Shader::SetUniformTexture3D(std::string name, GLuint textureid, GLuint activeTextureid)
	{
		SetUniformTexture(name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTURE3D);
	}

	void Shader::SetUniformTextureArray(std::string name, GLuint textureid, GLuint activeTextureid)
	{
		SetUniformTexture(name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTUREARRAY);
	}

	void Shader::SetUniformFloats(std::string name, int size, float* data, bool ismatrix)
	{
		float* input_uniform = new float[size];
		for (int i = 0; i < size; i++)
		{
			input_uniform[i] = data[i];
		}

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			GLSL_VAR_TYPES type;
			if (ismatrix)
			{
				if (size == 9)
					type = FLOAT3X3;
				else if (size == 16)
					type = FLOAT4X4;
			}
			else
			{
				if (size == 3)
					type = FLOAT3;
				else if (size == 4)
					type = FLOAT4;
			}
			ul.type = type;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
	}

	void Shader::SetGeometryShaderPrimitives(GLenum in, GLenum out)
	{
		m_geometryPrimitiveIn = in;
		m_geometryPrimitiveOut = out;
		int temp;
		glProgramParameteriEXT(m_shader_program, GL_GEOMETRY_INPUT_TYPE_EXT, in);
		glProgramParameteriEXT(m_shader_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, out);
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
		glProgramParameteriEXT(m_shader_program, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
	}

	void Shader::SetGeometryShaderPrimitives(GS_INPUT in, GS_OUTPUT out)
	{
		m_geometryPrimitiveIn = in;
		m_geometryPrimitiveOut = out;
		int temp;
		glProgramParameteriEXT(m_shader_program, GL_GEOMETRY_INPUT_TYPE_EXT, in);
		glProgramParameteriEXT(m_shader_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, out);
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
		glProgramParameteriEXT(m_shader_program, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
	}

	void Shader::SetUniformTexture(std::string name, GLuint textureid, GLuint activeTextureid, GLSL_VAR_TYPES type)
	{
		GLuint* input_uniform = new GLuint[2];
		input_uniform[0] = textureid;
		input_uniform[1] = activeTextureid;

		if (m_uniform_variables.find(name) != m_uniform_variables.end())
		{
			m_uniform_variables[name].DestroyValue();
			m_uniform_variables[name].value = input_uniform;
		}
		else
		{
			Shader::UniformVariable ul;
			ul.type = type;
			ul.m_loc = glGetUniformLocation(m_shader_program, name.c_str());
			ul.value = input_uniform;
			m_uniform_variables.insert(std::pair<std::string, Shader::UniformVariable>(name, ul));
		}
		//gl::ExitOnGLError("ERROR: Could not set shader Uniform");
	}
}