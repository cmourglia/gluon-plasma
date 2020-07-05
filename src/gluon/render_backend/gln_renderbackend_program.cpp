#include <gluon/render_backend/gln_renderbackend.h>

#include <glad/glad.h>

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>

#include <loguru.hpp>

#include <stdio.h>

namespace gln
{

constexpr GLenum k_ShaderTypes[ShaderType_Count] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_COMPUTE_SHADER,
};

shader_handle CreateShader(const char* ShaderSource, shader_type ShaderType, const char* ShaderName)
{
	shader_handle Handle = GLUON_INVALID_HANDLE;

	auto Shader = glCreateShader(k_ShaderTypes[ShaderType]);

	glShaderSource(Shader, 1, &ShaderSource, nullptr);
	glCompileShader(Shader);

	GLint Compiled;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Compiled);

	if (Compiled != GL_TRUE)
	{
		GLchar InfoLog[512];
		glGetShaderInfoLog(Shader, 512, nullptr, InfoLog);
		LOG_F(ERROR, "Could not compile Shader %s:\n%s", ShaderName, InfoLog);
		glDeleteShader(Shader);
	}
	else
	{
		Handle.Idx = Shader;
	}

	return Handle;
}

shader_handle LoadShader(const char* ShaderName, shader_type ShaderType)
{
	FILE* File = fopen(ShaderName, "r");
	if (!File)
	{
		LOG_F(ERROR, "Cannot open file %s", ShaderName);
		return GLUON_INVALID_HANDLE;
	}

	fseek(File, 0, SEEK_END);
	size_t Size = ftell(File);
	fseek(File, 0, SEEK_SET);

	char* Data = (char*)malloc(Size + 1);
	if (!Data)
	{
		LOG_F(ERROR, "No more memory available");
		fclose(File);
		return GLUON_INVALID_HANDLE;
	}

	Size       = fread(Data, sizeof(char), Size, File);
	Data[Size] = '\0';

	shader_handle Handle = CreateShader(Data, ShaderType, ShaderName);

	free(Data);
	fclose(File);

	return Handle;
}

program_handle CreateProgram(shader_handle VertexShader, shader_handle FragmentShader, bool DeleteShaders)
{
	program_handle Handle = GLUON_INVALID_HANDLE;

	uint32_t Program = glCreateProgram();

	if (glIsShader(VertexShader.Idx))
	{
		glAttachShader(Program, VertexShader.Idx);
	}

	if (glIsShader(FragmentShader.Idx))
	{
		glAttachShader(Program, FragmentShader.Idx);
	}

	glLinkProgram(Program);

	if (DeleteShaders)
	{
		if (glIsShader(VertexShader.Idx))
		{
			glDetachShader(Program, VertexShader.Idx);
			glDeleteShader(VertexShader.Idx);
		}

		if (glIsShader(FragmentShader.Idx))
		{
			glDetachShader(Program, FragmentShader.Idx);
			glDeleteShader(FragmentShader.Idx);
		}
	}

	GLint Linked;

	glGetProgramiv(Program, GL_LINK_STATUS, &Linked);

	if (Linked != GL_TRUE)
	{
		GLchar InfoLog[512];
		glGetProgramInfoLog(Program, 512, nullptr, InfoLog);

		LOG_F(ERROR, "Error linking program (%s, %s):\n%s", "vert", "frag", InfoLog);

		glDeleteProgram(Program);
	}
	else
	{
		Handle.Idx = Program;
	}

	return Handle;
}

}
