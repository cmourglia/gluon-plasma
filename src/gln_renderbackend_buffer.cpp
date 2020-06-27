#include "render_backend.h"

#include <glad/glad.h>

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>

#include <loguru.hpp>

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

vertex_array_handle CreateVertexArray(buffer_handle IndexBuffer)
{
	vertex_array_handle Handle;
	glCreateVertexArrays(1, &Handle.Idx);

	glVertexArrayElementBuffer(Handle.Idx, IndexBuffer.Idx);

	return Handle;
}

static eastl::map<vertex_array_handle, eastl::vector<buffer_handle>> g_VertexArrayAttachments;

void AttachVertexBuffer(vertex_array_handle VertexArray, buffer_handle VertexBuffer, const vertex_layout& VertexLayout)
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

	if (Size > 0)
	{
		glNamedBufferStorage(Result.Idx, Size, Data, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	}

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
	if (!glIsBuffer(Handle->Idx))
	{
		LOG_F(ERROR, "Buffer %d is not a buffer", Handle->Idx);
	}

	if (s_BufferInfos[Handle->Idx].Mapped)
	{
		LOG_F(ERROR, "Buffer %d must be unmapped before resize", Handle->Idx);
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
}
