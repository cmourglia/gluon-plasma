#include "render_backend.h"

#include <glad/glad.h>

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>

#include <loguru.hpp>

#include <stdio.h>

void OpenGLMessageCallback(GLenum        Source,
                           GLenum        Type,
                           GLuint        Id,
                           GLenum        Severity,
                           GLsizei       Length,
                           GLchar const* Message,
                           void const*   UserParam)
{
	const auto SourceStr = [Source]() {
		switch (Source)
		{
			case GL_DEBUG_SOURCE_API:
				return "API";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				return "WINDOW SYSTEM";
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				return "SHADER COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				return "THIRD PARTY";
			case GL_DEBUG_SOURCE_APPLICATION:
				return "APPLICATION";
			case GL_DEBUG_SOURCE_OTHER:
			default:
				return "OTHER";
		}
	}();

	switch (Type)
	{
		case GL_DEBUG_TYPE_ERROR:
			LOG_F(ERROR, "%s (%d): %s", SourceStr, Id, Message);
			break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			LOG_F(WARNING, "%s (%d) UB: %s", SourceStr, Id, Message);
			break;

		default:
			LOG_F(INFO, "%s (%d): %s", SourceStr, Id, Message);
	}
}

constexpr GLenum k_DataTypesToGL[DataType_Count] = {
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_INT,
    GL_UNSIGNED_INT,
    GL_FLOAT,
};

inline constexpr uint32_t GetDataTypeSize(data_type DataType)
{
	switch (DataType)
	{
		case DataType_Byte:
		case DataType_UnsignedByte:
			return 1;

		case DataType_Short:
		case DataType_UnsignedShort:
			return 2;

		case DataType_Int:
		case DataType_UnsignedInt:
		case DataType_Float:
			return 4;
	}

	return 0;
}

static eastl::unordered_map<vertex_array_handle, GLenum> g_VertexArrayIndexSizes;

struct vertex_layout_impl
{
	eastl::vector<data_type> DataTypes;
	eastl::vector<int>       ElemCounts;

	uint32_t GetOffset(uint32_t Index) const
	{
		uint32_t Offset = 0;

		for (uint32_t i = 0; i < Index; ++i)
		{
			Offset += ElemCounts[i] * GetDataTypeSize(DataTypes[i]);
		}

		return Offset;
	}

	uint32_t GetTotalSize() const
	{
		return GetOffset((uint32_t)DataTypes.size());
	}
};

vertex_layout::vertex_layout()
{
	m_Impl = new vertex_layout_impl;
}

vertex_layout::~vertex_layout()
{
	delete m_Impl;
	m_Impl = nullptr;
}

vertex_layout& vertex_layout::Begin()
{
	return *this;
}

vertex_layout& vertex_layout::Add(data_type DataType, int ElementCount)
{
	m_Impl->DataTypes.push_back(DataType);
	m_Impl->ElemCounts.push_back(ElementCount);
	return *this;
}

void vertex_layout::End()
{
	/* no-op for now */
}

struct draw_elements_command
{
	GLuint VertexCount;
	GLuint InstanceCount;
	GLuint FirstIndex;
	GLuint BaseVertex;
	GLuint BaseInstance;
};

void EnableDebugging()
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OpenGLMessageCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
}

void DisableDebugging()
{
	glDisable(GL_DEBUG_OUTPUT);
	glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}

vertex_array_handle CreateVertexArray(index_buffer_handle IndexBuffer)
{
	vertex_array_handle Handle;
	glCreateVertexArrays(1, &Handle.Idx);

	glVertexArrayElementBuffer(Handle.Idx, IndexBuffer.Idx);

	return Handle;
}

static eastl::map<vertex_array_handle, eastl::vector<vertex_buffer_handle>> g_VertexArrayAttachments;

void AttachVertexBuffer(vertex_array_handle VertexArray, vertex_buffer_handle VertexBuffer, const vertex_layout& VertexLayout)
{
	uint32_t AttachmentCount = (uint32_t)g_VertexArrayAttachments[VertexArray].size();

	glVertexArrayVertexBuffer(VertexArray.Idx, AttachmentCount, VertexBuffer.Idx, 0, VertexLayout.m_Impl->GetTotalSize());

	for (uint32_t i = 0; i < VertexLayout.m_Impl->DataTypes.size(); ++i)
	{
		const uint32_t  Count    = VertexLayout.m_Impl->ElemCounts[i];
		const data_type DataType = VertexLayout.m_Impl->DataTypes[i];

		glEnableVertexArrayAttrib(VertexArray.Idx, i);
		glVertexArrayAttribBinding(VertexArray.Idx, i, AttachmentCount);

		if (DataType == DataType_Float)
		{
			glVertexArrayAttribFormat(VertexArray.Idx, i, Count, k_DataTypesToGL[DataType], GL_FALSE, VertexLayout.m_Impl->GetOffset(i));
		}
		else
		{
			glVertexArrayAttribIFormat(VertexArray.Idx, i, Count, k_DataTypesToGL[DataType], VertexLayout.m_Impl->GetOffset(i));
		}
	}
}

vertex_buffer_handle CreateVertexBuffer(uint64_t Size, const void* Data)
{
	vertex_buffer_handle Handle;
	glCreateBuffers(1, &Handle.Idx);
	glNamedBufferStorage(Handle.Idx, Size, Data, GL_DYNAMIC_STORAGE_BIT);

	return Handle;
}

index_buffer_handle CreateIndexBuffer(uint64_t Size, const void* Data)
{
	index_buffer_handle Handle;
	glCreateBuffers(1, &Handle.Idx);
	glNamedBufferStorage(Handle.Idx, Size, Data, GL_DYNAMIC_STORAGE_BIT);

	return Handle;
}

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

struct buffer_info
{
	int64_t Size      = 0;
	bool    Mapped    = false;
	bool    Immutable = false;
};

static eastl::unordered_map<uint32_t, buffer_info> s_BufferInfos;

buffer_handle CreateBuffer(int64_t Size, const void* Data)
{
	buffer_handle Result;

	glCreateBuffers(1, &Result.Idx);

	if (Size >= 0)
	{
		glNamedBufferData(Result.Idx, Size, Data, GL_DYNAMIC_DRAW);
	}

	s_BufferInfos[Result.Idx] = {Size, false, false};

	return Result;
}

buffer_handle CreateImmutableBuffer(int64_t Size, const void* Data)
{
	buffer_handle Result;

	glCreateBuffers(1, &Result.Idx);

	glNamedBufferStorage(Result.Idx, Size, Data, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	s_BufferInfos[Result.Idx] = {Size, false, true};

	return Result;
}

void DestroyBuffer(buffer_handle Handle)
{
	buffer_info Infos = s_BufferInfos[Handle.Idx];

	UnmapBuffer(Handle);

#ifdef _DEBUG
	if (glIsBuffer(Handle.Idx))
	{
		glDeleteBuffers(1, &Handle.Idx);
	}
	else
	{
		LOG_F(ERROR, "Buffer %d is not a buffer", Handle.Idx);
	}
#else
	glDeleteBuffers(1, &Handle.Idx);
#endif

	s_BufferInfos.erase(Handle.Idx);
}

void ResizeBuffer(buffer_handle Handle, int64_t NewSize, const void* Data)
{
#ifdef _DEBUG
	if (!glIsBuffer(Handle.Idx))
	{
		LOG_F(ERROR, "Buffer %d is not a buffer", Handle.Idx);
	}

	if (s_BufferInfos[Handle.Idx].Mapped)
	{
		LOG_F(ERROR, "Buffer %d must be unmapped before resize", Handle.Idx);
	}
#endif

	glNamedBufferData(Handle.Idx, NewSize, Data, GL_DYNAMIC_DRAW);
	s_BufferInfos[Handle.Idx].Size = NewSize;
}

void ResizeImmutableBuffer(buffer_handle* Handle, int64_t NewSize, const void* Data)
{
#ifdef _DEBUG
	if (!glIsBuffer(Handle.Idx))
	{
		LOG_F(ERROR, "Buffer %d is not a buffer", Handle.Idx);
	}

	if (s_BufferInfos[Handle.Idx].Mapped)
	{
		LOG_F(ERROR, "Buffer %d must be unmapped before resize", Handle.Idx);
	}
#endif

	DestroyBuffer(*Handle);
	*Handle = CreateImmutableBuffer(NewSize, Data);
}

void* MapBuffer(buffer_handle Handle, int64_t Offset, int64_t Length)
{
	if (Length <= 0)
	{
		Length = s_BufferInfos[Handle.Idx].Size - Offset;
	}

	s_BufferInfos[Handle.Idx].Mapped = true;

	const GLenum Flags = s_BufferInfos[Handle.Idx].Immutable ? GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT
	                                                         : GL_MAP_WRITE_BIT;

	void* Result = glMapNamedBufferRange(Handle.Idx, Offset, Length, Flags);

	return Result;
}

void UnmapBuffer(buffer_handle Handle)
{
	if (s_BufferInfos[Handle.Idx].Mapped)
	{
		glUnmapNamedBuffer(Handle.Idx);
		s_BufferInfos[Handle.Idx].Mapped = false;
	}
}

void UpdateBufferData(buffer_handle Handle, const void* Data, int64_t Offset, int64_t Length)
{
	if (Length <= 0)
	{
		Length = s_BufferInfos[Handle.Idx].Size - Offset;
	}

	glNamedBufferSubData(Handle.Idx, Offset, Length, Data);
}
