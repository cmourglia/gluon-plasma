#include <gluon/render_backend/backend_opengl/gln_renderbackend_opengl.h>

#include <gluon/core/gln_math.h>

#include <glad/glad.h>
#include <EASTL/array.h>
#include <loguru.hpp>

#include <stdio.h>

namespace gln
{
namespace gl
{
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

	static const GLenum k_ShaderTypes[] = {
	    GL_VERTEX_SHADER,
	    GL_FRAGMENT_SHADER,
	    GL_COMPUTE_SHADER,
	};

	static const GLenum k_DataTypes[] = {
	    GL_BYTE,
	    GL_UNSIGNED_BYTE,
	    GL_SHORT,
	    GL_UNSIGNED_SHORT,
	    GL_INT,
	    GL_UNSIGNED_INT,
	    GL_FLOAT,
	};

	static const GLenum k_Formats[] = {
	    0,
	    GL_RED,
	    GL_RG,
	    GL_RGB,
	    GL_RGBA,
	};

	static const GLenum k_WrapModes[] = {
	    GL_CLAMP_TO_EDGE,
	    GL_CLAMP_TO_BORDER,
	    GL_MIRRORED_REPEAT,
	    GL_REPEAT,
	    GL_MIRROR_CLAMP_TO_EDGE,
	};

	static const GLenum k_MinFilters[] = {
	    GL_NEAREST,
	    GL_LINEAR,
	    GL_NEAREST_MIPMAP_NEAREST,
	    GL_LINEAR_MIPMAP_NEAREST,
	    GL_NEAREST_MIPMAP_LINEAR,
	    GL_LINEAR_MIPMAP_LINEAR,
	};

	static const GLenum k_MagFilters[] = {
	    GL_NEAREST,
	    GL_LINEAR,
	};

	static const GLenum k_InternalFormats[5][DataType_Count] = {
	    {0, 0, 0, 0, 0, 0, 0},
	    {GL_R8, GL_R8, GL_R16, GL_R16, GL_R32I, GL_R32UI, GL_R32F},
	    {GL_RG8, GL_RG8, GL_RG16, GL_RG16, GL_RG32I, GL_RG32UI, GL_RG32F},
	    {GL_RGB8, GL_RGB8, GL_RGB16, GL_RGB16, GL_RGB32I, GL_RGB32UI, GL_RGB32F},
	    {GL_RGBA8, GL_RGBA8, GL_RGBA16, GL_RGBA16, GL_RGBA32I, GL_RGBA32UI, GL_RGBA32F},
	};

	render_backend::render_backend() { }
	render_backend::~render_backend() { }

	// Misc section
	void render_backend::Initialize()
	{
		if (!gladLoadGL())
		{
			LOG_F(FATAL, "Cannot load OpenGL functions");
		}

		LOG_F(INFO, "OpenGL:\n\tVersion %s\n\tVendor %s", glGetString(GL_VERSION), glGetString(GL_VENDOR));
	}

	void render_backend::EnableDebugging()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
	}

	void render_backend::DisableDebugging()
	{
		glDisable(GL_DEBUG_OUTPUT);
		glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	// Shader section

	shader_handle render_backend::CreateShaderFromSource(const char* ShaderSource, shader_type ShaderType, const char* ShaderName)
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

	shader_handle render_backend::CreateShaderFromFile(const char* ShaderName, shader_type ShaderType)
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

		shader_handle Handle = CreateShaderFromSource(Data, ShaderType, ShaderName);

		free(Data);
		fclose(File);

		return Handle;
	}

	void render_backend::DestroyShader(shader_handle Shader)
	{
		GLN_ASSERT(glIsShader(Shader.Idx) && Shader.IsValid());
		glDeleteShader(Shader.Idx);
	}

	program_handle render_backend::CreateProgram(shader_handle VertexShader, shader_handle FragmentShader, bool DeleteShaders)
	{
		program_handle Handle = GLUON_INVALID_HANDLE;

		u32 Program = glCreateProgram();

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

	program_handle render_backend::CreateComputeProgram(shader_handle ComputeShader, bool DeleteShaders)
	{
		program_handle Handle = GLUON_INVALID_HANDLE;

		u32 Program = glCreateProgram();

		if (glIsShader(ComputeShader.Idx))
		{
			glAttachShader(Program, ComputeShader.Idx);
		}

		glLinkProgram(Program);

		if (DeleteShaders)
		{
			if (glIsShader(ComputeShader.Idx))
			{
				glDetachShader(Program, ComputeShader.Idx);
				glDeleteShader(ComputeShader.Idx);
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

	void render_backend::DestroyProgram(program_handle Program)
	{
		GLN_ASSERT(glIsProgram(Program.Idx) && Program.IsValid());
		glDeleteProgram(Program.Idx);
	}

	// TODO: Set uniforms

	// VAO section
	vertex_array_handle render_backend::CreateVertexArray(buffer_handle IndexBuffer)
	{
		vertex_array_handle Handle;
		glCreateVertexArrays(1, &Handle.Idx);

		glVertexArrayElementBuffer(Handle.Idx, IndexBuffer.Idx);

		return Handle;
	}

	void render_backend::AttachVertexBuffer(vertex_array_handle VertexArray, buffer_handle VertexBuffer, const vertex_layout& VertexLayout)
	{
		u32 AttachmentCount = (u32)m_VertexArrayAttachments[VertexArray].size();

		glVertexArrayVertexBuffer(VertexArray.Idx, AttachmentCount, VertexBuffer.Idx, 0, VertexLayout.GetTotalSize());

		for (u32 Index = 0; Index < VertexLayout.GetEntryCount(); ++Index)
		{
			auto Entry = VertexLayout.GetEntry(Index);

			glEnableVertexArrayAttrib(VertexArray.Idx, Index);
			glVertexArrayAttribBinding(VertexArray.Idx, Index, AttachmentCount);

			if (Entry.DataType == DataType_Float)
			{
				glVertexArrayAttribFormat(VertexArray.Idx,
				                          Index,
				                          Entry.ElementCount,
				                          k_DataTypes[Entry.DataType],
				                          GL_FALSE,
				                          VertexLayout.GetOffset(Index));
			}
			else
			{
				glVertexArrayAttribIFormat(VertexArray.Idx,
				                           Index,
				                           Entry.ElementCount,
				                           k_DataTypes[Entry.DataType],
				                           VertexLayout.GetOffset(Index));
			}
		}

		m_VertexArrayAttachments[VertexArray].push_back(VertexBuffer);
	}

	void render_backend::DestroyVertexArray(vertex_array_handle VertexArray)
	{
		GLN_ASSERT(VertexArray.IsValid() && glIsVertexArray(VertexArray.Idx));
		glDeleteVertexArrays(1, &VertexArray.Idx);
	}

	// Buffers section
	buffer_handle render_backend::CreateBuffer(i64 Size, const void* Data)
	{
		buffer_handle Result;

		glCreateBuffers(1, &Result.Idx);

		if (Size >= 0)
		{
			glNamedBufferData(Result.Idx, Size, Data, GL_DYNAMIC_DRAW);
		}

		m_BufferInfos[Result] = {Size, false, false};

		return Result;
	}

	buffer_handle render_backend::CreateImmutableBuffer(i64 Size, const void* Data)
	{
		buffer_handle Result;

		glCreateBuffers(1, &Result.Idx);

		if (Size > 0)
		{
			glNamedBufferStorage(Result.Idx, Size, Data, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		}

		m_BufferInfos[Result] = {Size, false, true};

		return Result;
	}

	void render_backend::ResizeBuffer(buffer_handle Handle, i64 NewSize, const void* Data)
	{
#ifdef _DEBUG
		if (!glIsBuffer(Handle.Idx))
		{
			LOG_F(ERROR, "Buffer %d is not a buffer", Handle.Idx);
		}

		if (m_BufferInfos[Handle].Mapped)
		{
			LOG_F(ERROR, "Buffer %d must be unmapped before resize", Handle.Idx);
		}
#endif

		glNamedBufferData(Handle.Idx, NewSize, Data, GL_DYNAMIC_DRAW);
		m_BufferInfos[Handle].Size = NewSize;
	}

	void render_backend::ResizeImmutableBuffer(buffer_handle* Buffer, i64 NewSize, const void* Data)
	{
#ifdef _DEBUG
		if (!glIsBuffer(Buffer->Idx))
		{
			LOG_F(ERROR, "Buffer %d is not a buffer", Buffer->Idx);
		}

		if (m_BufferInfos[*Buffer].Mapped)
		{
			LOG_F(ERROR, "Buffer %d must be unmapped before resize", Buffer->Idx);
		}
#endif

		DestroyBuffer(*Buffer);
		*Buffer = CreateImmutableBuffer(NewSize, Data);
	}

	void render_backend::UpdateBufferData(buffer_handle Buffer, const void* Data, i64 Offset, i64 Length)
	{
		if (Length <= 0)
		{
			Length = m_BufferInfos[Buffer].Size - Offset;
		}

		glNamedBufferSubData(Buffer.Idx, Offset, Length, Data);
	}

	void render_backend::DestroyBuffer(buffer_handle Buffer)
	{
		UnmapBuffer(Buffer);

#ifdef _DEBUG
		if (glIsBuffer(Buffer.Idx))
		{
			glDeleteBuffers(1, &Buffer.Idx);
		}
		else
		{
			LOG_F(ERROR, "Buffer %d is not a buffer", Buffer.Idx);
		}
#else
		glDeleteBuffers(1, &Buffer.Idx);
#endif

		m_BufferInfos.erase(Buffer);
	}

	void* render_backend::MapBuffer(buffer_handle Buffer, i64 Offset, i64 Length)
	{
		if (Length <= 0)
		{
			Length = m_BufferInfos[Buffer].Size - Offset;
		}

		m_BufferInfos[Buffer].Mapped = true;

		const GLenum Flags = m_BufferInfos[Buffer].Immutable ? GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT
		                                                     : GL_MAP_WRITE_BIT;

		void* Result = glMapNamedBufferRange(Buffer.Idx, Offset, Length, Flags);

		return Result;
	}

	void render_backend::UnmapBuffer(buffer_handle Buffer)
	{
		if (m_BufferInfos[Buffer].Mapped)
		{
			glUnmapNamedBuffer(Buffer.Idx);
			m_BufferInfos[Buffer].Mapped = false;
		}
	}

	// Texture section
	texture_handle render_backend::CreateTexture(u32       Width,
	                                             u32       Height,
	                                             u32       ComponentCount,
	                                             data_type DataType,
	                                             bool      WithMipmaps,
	                                             void*     Data)
	{
		texture_handle Texture;

		glCreateTextures(GL_TEXTURE_2D, 1, &Texture.Idx);

		u32 Levels = 1;
		if (WithMipmaps)
		{
			Levels = (u32)(log2f(Min((f32)Width, (f32)Height)));
		}

		glTextureStorage2D(Texture.Idx, Levels, k_InternalFormats[ComponentCount][DataType], Width, Height);

		SetTextureFiltering(Texture, MinFilter_Linear, MagFilter_Linear);
		SetTextureWrapping(Texture, WrapMode_Repeat, WrapMode_Repeat);

		m_TextureInfos[Texture] = {Width, Height, ComponentCount, DataType, WithMipmaps};

		if (Data != nullptr)
		{
			SetTextureData(Texture, Data);
		}

		return Texture;
	}

	void render_backend::SetTextureData(texture_handle Texture, void* Data)
	{
		const auto& Info = m_TextureInfos[Texture];
		glTextureSubImage2D(Texture.Idx,
		                    0,
		                    0,
		                    0,
		                    Info.Width,
		                    Info.Height,
		                    k_Formats[Info.ComponentCount],
		                    k_DataTypes[Info.DataType],
		                    Data);

		if (Info.WithMipmap)
		{
			glGenerateTextureMipmap(Texture.Idx);
		}
	}

	void render_backend::SetTextureWrapping(texture_handle Texture, wrap_mode WrapS, wrap_mode WrapT)
	{
		glTextureParameteri(Texture.Idx, GL_TEXTURE_WRAP_S, k_WrapModes[WrapS]);
		glTextureParameteri(Texture.Idx, GL_TEXTURE_WRAP_T, k_WrapModes[WrapT]);
	}

	void render_backend::SetTextureFiltering(texture_handle Texture, min_filter MinFilter, mag_filter MagFilter)
	{
		glTextureParameteri(Texture.Idx, GL_TEXTURE_MIN_FILTER, k_MinFilters[MinFilter]);
		glTextureParameteri(Texture.Idx, GL_TEXTURE_MAG_FILTER, k_MagFilters[MagFilter]);
	}

	void render_backend::DestroyTexture(texture_handle Texture)
	{
		GLN_ASSERT(Texture.IsValid() && glIsTexture(Texture.Idx));
		glDeleteTextures(1, &Texture.Idx);
	}
}
}
