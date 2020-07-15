#pragma once

#include <gluon/core/gln_defines.h>

#include <gluon/render_backend/gln_renderbackend.h>

#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>

namespace eastl
{
template <>
struct hash<gluon::shader_handle>
{
	size_t operator()(gluon::shader_handle Handle) const { return Handle.Idx; }
};

template <>
struct hash<gluon::program_handle>
{
	size_t operator()(gluon::program_handle Handle) const { return Handle.Idx; }
};

template <>
struct hash<gluon::vertex_array_handle>
{
	size_t operator()(gluon::vertex_array_handle Handle) const { return Handle.Idx; }
};

template <>
struct hash<gluon::buffer_handle>
{
	size_t operator()(gluon::buffer_handle Handle) const { return Handle.Idx; }
};

template <>
struct hash<gluon::texture_handle>
{
	size_t operator()(gluon::texture_handle Handle) const { return Handle.Idx; }
};
}

/// This is a private header, it should not be included outside of the gluon renderbackend files.
namespace gluon
{
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

		default:
			return 0;
	}

	return 0;
}

struct GLN_NO_VTABLE render_backend_interface
{
	// Misc section
	virtual void Initialize() = 0;

	virtual void EnableDebugging()  = 0;
	virtual void DisableDebugging() = 0;

	// Shader section
	virtual shader_handle CreateShaderFromSource(const char* ShaderSource, shader_type ShaderType, const char* ShaderName) = 0;
	virtual shader_handle CreateShaderFromFile(const char* ShaderName, shader_type ShaderType)                             = 0;
	virtual void          DestroyShader(shader_handle Shader)                                                              = 0;

	virtual program_handle CreateProgram(shader_handle VertexShader, shader_handle FragmentShader, bool DeleteShaders) = 0;
	virtual program_handle CreateComputeProgram(shader_handle ComputeShader, bool DeleteShaders)                       = 0;
	virtual void           SetProgram(program_handle Program)                                                          = 0;
	virtual void           DestroyProgram(program_handle Program)                                                      = 0;

	// TODO: This uniform handling does not fit DirectX or Vulkan paradigms
	virtual void SetUniform(const char* UniformName, i32 Value)         = 0;
	virtual void SetUniform(const char* UniformName, u32 Value)         = 0;
	virtual void SetUniform(const char* UniformName, f32 Value)         = 0;
	virtual void SetUniform(const char* UniformName, const vec2& Value) = 0;
	virtual void SetUniform(const char* UniformName, const vec3& Value) = 0;
	virtual void SetUniform(const char* UniformName, const vec4& Value) = 0;
	virtual void SetUniform(const char* UniformName, const mat2& Value) = 0;
	virtual void SetUniform(const char* UniformName, const mat3& Value) = 0;
	virtual void SetUniform(const char* UniformName, const mat4& Value) = 0;

	// VAO section
	virtual vertex_array_handle CreateVertexArray(buffer_handle IndexBuffer)                                                        = 0;
	virtual void AttachVertexBuffer(vertex_array_handle VertexArray, buffer_handle VertexBuffer, const vertex_layout& VertexLayout) = 0;
	virtual void DestroyVertexArray(vertex_array_handle VertexArray)                                                                = 0;

	// Buffers section
	virtual buffer_handle CreateBuffer(i64 Size, const void* Data)                                         = 0;
	virtual buffer_handle CreateImmutableBuffer(i64 Size, const void* Data)                                = 0;
	virtual void          ResizeBuffer(buffer_handle Handle, i64 NewSize, const void* Data)                = 0;
	virtual void          ResizeImmutableBuffer(buffer_handle* Handle, i64 NewSize, const void* Data)      = 0;
	virtual void          UpdateBufferData(buffer_handle Handle, const void* Data, i64 Offset, i64 Length) = 0;
	virtual void          DestroyBuffer(buffer_handle Buffer)                                              = 0;

	virtual void* MapBuffer(buffer_handle Handle, i64 Offset, i64 Length) = 0;
	virtual void  UnmapBuffer(buffer_handle Handle)                       = 0;

	// Texture section
	virtual texture_handle CreateTexture(u32 Width, u32 Height, u32 ComponentCount, data_type DataType, bool WithMipmaps, void* Data) = 0;
	virtual void           SetTextureData(texture_handle Texture, void* Data)                                                         = 0;
	virtual void           SetTextureWrapping(texture_handle Texture, wrap_mode WrapS, wrap_mode WrapT)                               = 0;
	virtual void           SetTextureFiltering(texture_handle Texture, min_filter MinFilter, mag_filter MagFilter)                    = 0;
	virtual void           DestroyTexture(texture_handle Texture)                                                                     = 0;
};
}
