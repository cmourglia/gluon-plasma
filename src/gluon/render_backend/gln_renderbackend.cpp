#include <gluon/render_backend/gln_renderbackend.h>

#include <gluon/render_backend/gln_renderbackend_p.h>
#include <gluon/render_backend/backend_opengl/gln_renderbackend_opengl.h>

#include <EASTL/vector.h>

namespace gln
{

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

	uint32_t GetTotalSize() const { return GetOffset((uint32_t)DataTypes.size()); }
};

vertex_layout::vertex_layout() { m_Impl = new vertex_layout_impl; }

vertex_layout::~vertex_layout()
{
	delete m_Impl;
	m_Impl = nullptr;
}

vertex_layout& vertex_layout::Begin() { return *this; }

vertex_layout& vertex_layout::Add(data_type DataType, int ElementCount)
{
	m_Impl->DataTypes.push_back(DataType);
	m_Impl->ElemCounts.push_back(ElementCount);
	return *this;
}

void vertex_layout::End()
{ /* no-op for now */
}

u32 vertex_layout::GetTotalSize() const { return m_Impl->GetTotalSize(); }

u32 vertex_layout::GetOffset(u32 Index) const { return m_Impl->GetOffset(Index); }

u32 vertex_layout::GetEntryCount() const { return (u32)m_Impl->DataTypes.size(); }

vertex_layout::entry vertex_layout::GetEntry(u32 Index) const { return {m_Impl->DataTypes[Index], m_Impl->ElemCounts[Index]}; }

static render_backend_interface* s_Backend = nullptr;

void InitializeBackend()
{
	s_Backend = new gl::render_backend();
	s_Backend->Initialize();
}

void EnableDebugging() { s_Backend->EnableDebugging(); }
void DisableDebugging() { s_Backend->DisableDebugging(); }

shader_handle CreateShaderFromSource(const char* ShaderSource, shader_type ShaderType, const char* ShaderName)
{
	return s_Backend->CreateShaderFromSource(ShaderSource, ShaderType, ShaderName);
}

shader_handle CreateShaderFromFile(const char* ShaderName, shader_type ShaderType)
{
	return s_Backend->CreateShaderFromFile(ShaderName, ShaderType);
}

void DestroyShader(shader_handle Shader) { s_Backend->DestroyShader(Shader); }

program_handle CreateProgram(shader_handle VertexShader, shader_handle FragmentShader, bool DeleteShaders)
{
	return s_Backend->CreateProgram(VertexShader, FragmentShader, DeleteShaders);
}

program_handle CreateComputeProgram(shader_handle ComputeShader, bool DeleteShader)
{
	return s_Backend->CreateComputeProgram(ComputeShader, DeleteShader);
}

void DestroyProgram(program_handle Program) { s_Backend->DestroyProgram(Program); }

vertex_array_handle CreateVertexArray(buffer_handle IndexBuffer) { return s_Backend->CreateVertexArray(IndexBuffer); }
void                AttachVertexBuffer(vertex_array_handle VertexArray, buffer_handle VertexBuffer, const vertex_layout& VertexLayout)
{
	s_Backend->AttachVertexBuffer(VertexArray, VertexBuffer, VertexLayout);
}
void DestroyVertexArray(vertex_array_handle VertexArray) { s_Backend->DestroyVertexArray(VertexArray); }

buffer_handle CreateBuffer(i64 Size, const void* Data) { return s_Backend->CreateBuffer(Size, Data); }
buffer_handle CreateImmutableBuffer(i64 Size, const void* Data) { return s_Backend->CreateImmutableBuffer(Size, Data); }
void          DestroyBuffer(buffer_handle Handle) { s_Backend->DestroyBuffer(Handle); }

void ResizeBuffer(buffer_handle Handle, i64 NewSize, const void* Data) { s_Backend->ResizeBuffer(Handle, NewSize, Data); }
void ResizeImmutableBuffer(buffer_handle* Handle, i64 NewSize, const void* Data)
{
	s_Backend->ResizeImmutableBuffer(Handle, NewSize, Data);
}

void UpdateBufferData(buffer_handle Handle, const void* Data, i64 Offset, i64 Length)
{
	s_Backend->UpdateBufferData(Handle, Data, Offset, Length);
}

void* MapBuffer(buffer_handle Handle, i64 Offset, i64 Length) { return s_Backend->MapBuffer(Handle, Offset, Length); }
void  UnmapBuffer(buffer_handle Handle) { return s_Backend->UnmapBuffer(Handle); }

texture_handle CreateTexture(u32 Width, u32 Height, u32 ComponentCount, data_type DataType, bool WithMipmaps, void* Data)
{
	return s_Backend->CreateTexture(Width, Height, ComponentCount, DataType, WithMipmaps, Data);
}

void SetTextureData(texture_handle Handle, void* Data) { s_Backend->SetTextureData(Handle, Data); }
void SetTextureWrapping(texture_handle Handle, wrap_mode WrapS, wrap_mode WrapT) { s_Backend->SetTextureWrapping(Handle, WrapS, WrapT); }
void SetTextureFiltering(texture_handle Handle, min_filter MinFilter, mag_filter MagFilter)
{
	s_Backend->SetTextureFiltering(Handle, MinFilter, MagFilter);
}
void DestroyTexture(texture_handle Handle) { s_Backend->DestroyTexture(Handle); }

}
