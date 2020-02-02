#pragma once

#include <inttypes.h>

#include <EASTL/numeric_limits.h>

static constexpr uint32_t k_InvalidHandle = eastl::numeric_limits<uint32_t>::max();

#define GLUON_HANDLE(name_t)                                                                                                               \
	struct name_t                                                                                                                          \
	{                                                                                                                                      \
		uint32_t Idx;                                                                                                                      \
	};                                                                                                                                     \
	inline bool IsValid(name_t Handle)                                                                                                     \
	{                                                                                                                                      \
		return Handle.Idx != k_InvalidHandle;                                                                                              \
	}

#define GLUON_INVALID_HANDLE                                                                                                               \
	{                                                                                                                                      \
		k_InvalidHandle                                                                                                                    \
	}

GLUON_HANDLE(shader_handle);
GLUON_HANDLE(program_handle);
GLUON_HANDLE(uniform_handle);

GLUON_HANDLE(vertex_array_handle);
GLUON_HANDLE(vertex_buffer_handle);
GLUON_HANDLE(index_buffer_handle);

GLUON_HANDLE(buffer_handle);

struct vertex_layout_impl;
class vertex_layout
{
public:
	vertex_layout& Begin();

	void End();

private:
	vertex_layout_impl* m_Impl;
};

vertex_array_handle  CreateVertexArray(vertex_buffer_handle VertexBuffer, vertex_layout VertexLayout, index_buffer_handle IndexBuffer);
vertex_buffer_handle CreateVertexBuffer();
index_buffer_handle  CreateIndexBuffer();
