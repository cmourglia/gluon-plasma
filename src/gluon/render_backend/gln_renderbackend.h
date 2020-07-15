#pragma once

#include <gluon/core/gln_defines.h>

#include <EASTL/numeric_limits.h>

#ifdef _WIN32
#	ifdef GLUON_RENDERBACKEND_MAKELIB
#		define GLUON_RENDERBACKEND_EXPORT
#	else
#		ifdef GLUON_RENDERBACKEND_MAKEDLL
#			define GLUON_RENDERBACKEND_EXPORT __declspec(dllexport)
#		else
#			define GLUON_RENDERBACKEND_EXPORT __declspec(dllimport)
#		endif
#	endif
#else
#	define GLUON_RENDERBACKEND_EXPORT
#endif

namespace gluon
{
static constexpr u32 k_InvalidHandle = UINT32_MAX;

#define GLUON_HANDLE(name_t)                                                                                                               \
	struct name_t                                                                                                                          \
	{                                                                                                                                      \
		u32         Idx;                                                                                                                   \
		inline bool IsValid() const { return Idx != k_InvalidHandle; }                                                                     \
	};                                                                                                                                     \
	inline bool operator<(const name_t& Left, const name_t& Right) { return Left.Idx < Right.Idx; }                                        \
	inline bool operator<=(const name_t& Left, const name_t& Right) { return Left.Idx <= Right.Idx; }                                      \
	inline bool operator==(const name_t& Left, const name_t& Right) { return Left.Idx == Right.Idx; }                                      \
	inline bool operator>=(const name_t& Left, const name_t& Right) { return Left.Idx >= Right.Idx; }                                      \
	inline bool operator>(const name_t& Left, const name_t& Right) { return Left.Idx > Right.Idx; }                                        \
	inline bool operator!=(const name_t& Left, const name_t& Right) { return Left.Idx != Right.Idx; }

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
struct GLUON_RENDERBACKEND_EXPORT vertex_layout
{
	struct entry
	{
		data_type DataType;
		i32       ElementCount;
	};

	vertex_layout();
	~vertex_layout();

	vertex_layout& Begin();
	vertex_layout& Add(data_type DataType, i32 ElementCount);
	void           End();

	u32 GetTotalSize() const;
	u32 GetOffset(u32 Index) const;

	u32   GetEntryCount() const;
	entry GetEntry(u32 Index) const;

	vertex_layout_impl* m_Impl;
};

GLUON_RENDERBACKEND_EXPORT void InitializeBackend();

GLUON_RENDERBACKEND_EXPORT void EnableDebugging();
GLUON_RENDERBACKEND_EXPORT void DisableDebugging();

GLUON_RENDERBACKEND_EXPORT shader_handle CreateShaderFromSource(const char* ShaderSource,
                                                                shader_type ShaderType,
                                                                const char* ShaderName = nullptr);
GLUON_RENDERBACKEND_EXPORT shader_handle CreateShaderFromFile(const char* ShaderName, shader_type ShaderType);
GLUON_RENDERBACKEND_EXPORT void          DestroyShader(shader_handle Shader);

GLUON_RENDERBACKEND_EXPORT program_handle CreateProgram(shader_handle VertexShader,
                                                        shader_handle FragmentShader = GLUON_INVALID_HANDLE,
                                                        bool          DeleteShaders  = false);
GLUON_RENDERBACKEND_EXPORT program_handle CreateComputeProgram(shader_handle ComputeShader, bool DeleteShader = false);

GLUON_RENDERBACKEND_EXPORT void DestroyProgram(program_handle Program);

GLUON_RENDERBACKEND_EXPORT vertex_array_handle CreateVertexArray(buffer_handle IndexBuffer);
GLUON_RENDERBACKEND_EXPORT void                AttachVertexBuffer(vertex_array_handle  VertexArray,
                                                                  buffer_handle        VertexBuffer,
                                                                  const vertex_layout& VertexLayout);
GLUON_RENDERBACKEND_EXPORT void                DestroyVertexArray(vertex_array_handle VertexArray);

GLUON_RENDERBACKEND_EXPORT buffer_handle CreateBuffer(i64 Size = -1, const void* Data = nullptr);
GLUON_RENDERBACKEND_EXPORT buffer_handle CreateImmutableBuffer(i64 Size, const void* Data = nullptr);
GLUON_RENDERBACKEND_EXPORT void          DestroyBuffer(buffer_handle Handle);

//! Do not resize an immutable buffer
//! Do not forget to unmap a buffer before resizing
GLUON_RENDERBACKEND_EXPORT void ResizeBuffer(buffer_handle Handle, i64 NewSize, const void* Data = nullptr);
GLUON_RENDERBACKEND_EXPORT void ResizeImmutableBuffer(buffer_handle* Handle, i64 NewSize, const void* Data = nullptr);

//! The buffer must be able to fit the whole data, UB otherwise
//! Set length to -1 to update the buffer until the end
GLUON_RENDERBACKEND_EXPORT void UpdateBufferData(buffer_handle Handle, const void* Data, i64 Offet = 0, i64 Length = -1);

//! No need to unmap buffers everytime, mapping is persistent and coherent.
//! Set length to -1 to map the buffer until the end
GLUON_RENDERBACKEND_EXPORT void* MapBuffer(buffer_handle Handle, i64 Offset = 0, i64 Length = -1);
GLUON_RENDERBACKEND_EXPORT void  UnmapBuffer(buffer_handle Handle);

GLUON_RENDERBACKEND_EXPORT texture_handle CreateTexture(u32       Width,
                                                        u32       Height,
                                                        u32       ComponentCount = 4,
                                                        data_type DataType       = DataType_UnsignedByte,
                                                        bool      WithMipmaps    = false,
                                                        void*     Data           = nullptr);

GLUON_RENDERBACKEND_EXPORT void SetTextureData(texture_handle Handle, void* Data);
GLUON_RENDERBACKEND_EXPORT void SetTextureWrapping(texture_handle Handle, wrap_mode WrapS, wrap_mode WrapT);
GLUON_RENDERBACKEND_EXPORT void SetTextureFiltering(texture_handle Handle, min_filter MinFilter, mag_filter MagFilter);
GLUON_RENDERBACKEND_EXPORT void DestroyTexture(texture_handle Handle);
}
