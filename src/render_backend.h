#pragma once

#include "gln_defines.h"

#include <EASTL/numeric_limits.h>

namespace gln
{
static constexpr uint32_t k_InvalidHandle = eastl::numeric_limits<uint32_t>::max();

#define GLUON_HANDLE(name_t)                                                                                                               \
	struct name_t                                                                                                                          \
	{                                                                                                                                      \
		uint32_t Idx;                                                                                                                      \
	};                                                                                                                                     \
	inline bool IsValid(name_t Handle) { return Handle.Idx != k_InvalidHandle; }                                                           \
	inline bool operator<(const name_t& Left, const name_t& Right) { return Left.Idx < Right.Idx; }

#define GLUON_INVALID_HANDLE                                                                                                               \
	{                                                                                                                                      \
		k_InvalidHandle                                                                                                                    \
	}

GLUON_HANDLE(shader_handle);
GLUON_HANDLE(program_handle);
GLUON_HANDLE(uniform_handle);

GLUON_HANDLE(vertex_array_handle);
GLUON_HANDLE(buffer_handle);

GLUON_HANDLE(texture_handle);

enum data_type
{
	DataType_Byte = 0,
	DataType_UnsignedByte,
	DataType_Short,
	DataType_UnsignedShort,
	DataType_Int,
	DataType_UnsignedInt,
	DataType_Float,
	DataType_Count,
};

enum shader_type
{
	ShaderType_Vertex = 0,
	ShaderType_Fragment,
	ShaderType_Geometry,
	ShaderType_TesselationControl,
	ShaderType_TesselationEvaluation,
	ShaderType_Compute,
	ShaderType_Count,
};

enum wrap_mode
{
	WrapMode_ClampToEdge = 0,
	WrapMode_ClampToBorder,
	WrapMode_MirroredRepeat,
	WrapMode_Repeat,
	WrapMode_MirrorClampToEdge,
	WrapMode_Count,
};

enum min_filter
{
	MinFilter_Nearest = 0,
	MinFilter_Linear,
	MinFilter_NearestMipmapNearest,
	MinFilter_LinearMipmapNearest,
	MinFilter_NearestMipmapLinear,
	MinFilter_LinearMipmapLinear,
	MinFilter_Count,
};

enum mag_filter
{
	MagFilter_Nearest = 0,
	MagFilter_Linear,
	MagFilter_Count,
};

struct vertex_layout_impl;
struct vertex_layout
{
	vertex_layout();
	~vertex_layout();

	vertex_layout& Begin();
	vertex_layout& Add(data_type DataType, int ElementCount);
	void           End();

	vertex_layout_impl* m_Impl;
};

void EnableDebugging();
void DisableDebugging();

shader_handle  CreateShader(const char* ShaderSource, shader_type ShaderType, const char* ShaderName = nullptr);
shader_handle  LoadShader(const char* ShaderName, shader_type ShaderType);
program_handle CreateProgram(shader_handle VertexShader, shader_handle FragmentShader = GLUON_INVALID_HANDLE, bool DeleteShaders = false);

void DestroyProgram(program_handle Program);

vertex_array_handle CreateVertexArray(buffer_handle IndexBuffer);
void                AttachVertexBuffer(vertex_array_handle VertexArray, buffer_handle VertexBuffer, const vertex_layout& VertexLayout);
void                DestroyVertexArray(vertex_array_handle VertexArray);

buffer_handle CreateVertexBuffer(u64 Size, const void* Data = nullptr);
buffer_handle CreateIndexBuffer(u64 Size, const void* Data = nullptr);

buffer_handle CreateBuffer(i64 Size = -1, const void* Data = nullptr);
buffer_handle CreateImmutableBuffer(i64 Size, const void* Data = nullptr);
void          DestroyBuffer(buffer_handle Handle);

//! Do not resize an immutable buffer
//! Do not forget to unmap a buffer before resizing
void ResizeBuffer(buffer_handle Handle, i64 NewSize, const void* Data = nullptr);
void ResizeImmutableBuffer(buffer_handle* Handle, i64 NewSize, const void* Data = nullptr);

//! The buffer must be able to fit the whole data, UB otherwise
//! Set length to -1 to update the buffer until the end
void UpdateBufferData(buffer_handle Handle, const void* Data, i64 Offet = 0, i64 Length = -1);

//! No need to unmap buffers everytime, mapping is persistent and coherent.
//! Set length to -1 to map the buffer until the end
void* MapBuffer(buffer_handle Handle, i64 Offset = 0, i64 Length = -1);
void  UnmapBuffer(buffer_handle Handle);

texture_handle CreateTexture(u32       Width,
                             u32       Height,
                             u32       ComponentCount = 4,
                             data_type DataType       = DataType_UnsignedByte,
                             bool      WithMipmaps    = false,
                             void*     Data           = nullptr);

void SetTextureData(texture_handle Handle, void* Data);
void SetTextureWrapping(texture_handle Handle, wrap_mode WrapS, wrap_mode WrapT);
void SetTextureFiltering(texture_handle Handle, min_filter MinFilter, mag_filter MagFilter);
void DestroyTexture(texture_handle Handle);
}
