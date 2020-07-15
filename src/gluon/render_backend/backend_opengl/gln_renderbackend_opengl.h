#pragma once

#include <gluon/render_backend/gln_renderbackend_p.h>

#include <EASTL/unordered_map.h>
#include <EASTL/vector.h>
#include <EASTL/string_hash_map.h>

namespace gluon
{
namespace gl
{
	struct program_info
	{
		eastl::string_hash_map<i32> UniformLocations;
	};

	struct buffer_info
	{
		int64_t Size      = 0;
		bool    Mapped    = false;
		bool    Immutable = false;
	};

	struct texture_info
	{
		u32       Width, Height;
		u32       ComponentCount;
		data_type DataType;
		bool      WithMipmap;
	};

	struct render_backend : public render_backend_interface
	{
		render_backend();
		virtual ~render_backend();

		// Misc section
		void Initialize() override final;

		void EnableDebugging() override final;
		void DisableDebugging() override final;

		// Shader section
		shader_handle CreateShaderFromSource(const char* ShaderSource, shader_type ShaderType, const char* ShaderName) override final;
		shader_handle CreateShaderFromFile(const char* ShaderName, shader_type ShaderType) override final;
		void          DestroyShader(shader_handle Shader) override final;

		program_handle CreateProgram(shader_handle VertexShader, shader_handle FragmentShader, bool DeleteShaders) override final;
		program_handle CreateComputeProgram(shader_handle ComputeShader, bool DeleteShaders) override final;
		void           SetProgram(program_handle Program) override final;
		void           DestroyProgram(program_handle Program) override final;

		void SetUniform(const char* UniformName, i32 Value) override final;
		void SetUniform(const char* UniformName, u32 Value) override final;
		void SetUniform(const char* UniformName, f32 Value) override final;
		void SetUniform(const char* UniformName, const vec2& Value) override final;
		void SetUniform(const char* UniformName, const vec3& Value) override final;
		void SetUniform(const char* UniformName, const vec4& Value) override final;
		void SetUniform(const char* UniformName, const mat2& Value) override final;
		void SetUniform(const char* UniformName, const mat3& Value) override final;
		void SetUniform(const char* UniformName, const mat4& Value) override final;

		// VAO section
		vertex_array_handle CreateVertexArray(buffer_handle IndexBuffer) override final;
		void                AttachVertexBuffer(vertex_array_handle  VertexArray,
		                                       buffer_handle        VertexBuffer,
		                                       const vertex_layout& VertexLayout) override final;
		void                DestroyVertexArray(vertex_array_handle VertexArray) override final;

		// Buffers section
		buffer_handle CreateBuffer(i64 Size, const void* Data) override final;
		buffer_handle CreateImmutableBuffer(i64 Size, const void* Data) override final;
		void          ResizeBuffer(buffer_handle Handle, i64 NewSize, const void* Data) override final;
		void          ResizeImmutableBuffer(buffer_handle* Handle, i64 NewSize, const void* Data) override final;
		void          UpdateBufferData(buffer_handle Handle, const void* Data, i64 Offset, i64 Length) override final;
		void          DestroyBuffer(buffer_handle Buffer) override final;

		void* MapBuffer(buffer_handle Handle, i64 Offset, i64 Length) override final;
		void  UnmapBuffer(buffer_handle Handle) override final;

		// Texture section
		texture_handle CreateTexture(u32 Width, u32 Height, u32 ComponentCount, data_type DataType, bool WithMipmaps, void* Data)
		    override final;
		void SetTextureData(texture_handle Texture, void* Data) override final;
		void SetTextureWrapping(texture_handle Texture, wrap_mode WrapS, wrap_mode WrapT) override final;
		void SetTextureFiltering(texture_handle Texture, min_filter MinFilter, mag_filter MagFilter) override final;
		void DestroyTexture(texture_handle Texture) override final;

		program_handle m_CurrentProgram;

		eastl::unordered_map<shader_handle, eastl::string>                      m_ShaderNames;
		eastl::unordered_map<program_handle, program_info>                      m_ProgramInfos;
		eastl::unordered_map<buffer_handle, buffer_info>                        m_BufferInfos;
		eastl::unordered_map<vertex_array_handle, eastl::vector<buffer_handle>> m_VertexArrayAttachments;
		eastl::unordered_map<texture_handle, texture_info>                      m_TextureInfos;
	};
}
}
