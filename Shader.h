#pragma once
/**
* Author: Leonardo Quatrin Campagnolo
* Date: July 2017
*
* OpenGL Shader Class implementation (Adapted)
* - Reload support
* - Attachment of a list of shaders
* - Geometry shader
*
* References:
* . Implementation based on Nvidia dual depth peeling sample from Louis Bavoli ('GLSLProgramObject')
*   -> http://developer.download.nvidia.com/SDK/10/opengl/screenshots/samples/dual_depth_peeling.html
*
* . Shader implementation by Cesar Tadeu Pozzer
*   -> http://www-usr.inf.ufsm.br/~pozzer/
*
* . Thanks to Eduardo Ceretta Dalla Favera for advices about shader reload.
**/

#ifndef GL_SHADER_H
#define GL_SHADER_H

#include <glm/glm.hpp>
#include <vector>

#include <map>
#define GLEW_STATIC
#include <GL/glew.h>

//Matrix has Comlunm major order
namespace gl
{
	enum GLSL_VAR_TYPES
	{
		UINT,
		INT,
		FLOAT,
		DOUBLE,
		GLMVEC2,
		GLMVEC3,
		GLMVEC4,
		GLMMAT4,
		VECTOR2,
		VECTOR3,
		FLOAT3,
		VECTOR4,
		FLOAT4,
		MATRIX2,
		MATRIX3,
		FLOAT3X3,
		MATRIX4,
		FLOAT4X4,
		TEXTURE1D,
		TEXTURE2D,
		TEXTURE3D,
		TEXTURERECTANGLE,
		TEXTUREARRAY,
		NONE,
	};

	enum SHADER_TYPE
	{
		VERTEX = 0,
		FRAGMENT = 1,
		GEOMETRY = 2,
	};

	class Shader
	{
	private:
		class UniformVariable
		{
		public:
			UniformVariable();
			~UniformVariable();

			void Bind();
			void DestroyValue();

			void* value;
			GLSL_VAR_TYPES type;
			GLuint m_loc;
		};

	public:
		static void Unbind();
		static char* TextFileRead(const char* file_name);
		static void CompileShader(GLuint shader, std::string filename);

		Shader();
		Shader(std::string vert, std::string frag);
		Shader(char *vert, char *frag, char* geom = NULL);
		~Shader();

		bool AddShaderFile(SHADER_TYPE type, std::string filename);

		bool LoadAndLink();

		void Bind();

		void Clear(bool delete_program = true);

		void Reload();

		GLint GetUniformLoc(char* name);
		GLint GetAttribLoc(char* name);

		GLuint GetProgramID();

		////////////////////////
		// Uniforms functions //
		////////////////////////
		void BindUniforms();
		void ClearUniforms();
		void BindUniform(std::string var_name);
		void SetUniform(std::string name, unsigned int value);
		void SetUniform(std::string name, int value);
		void SetUniform(std::string name, float value);
		void SetUniform(std::string name, double value);
		void SetUniform(std::string name, glm::vec2 value);
		void SetUniform(std::string name, glm::vec3 value);
		void SetUniform(std::string name, glm::vec4 value);
		void SetUniform(std::string name, glm::mat4 value);

		void SetUniformTexture1D(std::string name, GLuint textureid, GLuint activeTextureid);
		void SetUniformTexture2D(std::string name, GLuint textureid, GLuint activeTextureid);
		void SetUniformTextureRectangle(std::string name, GLuint textureid, GLuint activeTextureid);
		void SetUniformTexture3D(std::string name, GLuint textureid, GLuint activeTextureid);
		void SetUniformTextureArray(std::string name, GLuint textureid, GLuint activeTextureid);

		void SetUniformFloats(std::string name, int size, float* data, bool ismatrix);

		enum GS_INPUT
		{
			IN_POINTS = GL_POINTS,
			IN_LINES = GL_LINES,
			IN_LINES_ADJACENCY = GL_LINES_ADJACENCY,
			IN_TRIANGLES = GL_TRIANGLES,
			IN_TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY
		};

		enum GS_OUTPUT
		{
			OUT_POINTS = GL_POINTS,
			OUT_LINE_STRIP = GL_LINE_STRIP,
			OUT_TRIANGLE_STRIP = GL_TRIANGLE_STRIP
		};

		void SetGeometryShaderPrimitives(GLenum in, GLenum out);
		void SetGeometryShaderPrimitives(Shader::GS_INPUT in, Shader::GS_OUTPUT out);

	private:
		std::vector<std::string> vec_vertex_shaders_names;
		std::vector<std::string> vec_fragment_shaders_names;
		std::vector<std::string> vec_geometry_shaders_names;

		std::vector<GLuint> vec_vertex_shaders_ids;
		std::vector<GLuint> vec_fragment_shaders_ids;
		std::vector<GLuint> vec_geometry_shaders_ids;


		GLuint m_shader_program;
		GLenum m_geometryPrimitiveIn;
		GLenum m_geometryPrimitiveOut;

		std::map<std::string, Shader::UniformVariable> m_uniform_variables;

		bool m_hasGeometryShader;

		void SetUniformTexture(std::string name, GLuint textureid, GLuint activeTextureid, GLSL_VAR_TYPES type);
	};
}

#endif