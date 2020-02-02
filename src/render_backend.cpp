#include "render_backend.h"

#include <glad/glad.h>

#include <loguru.hpp>

vertex_array_handle CreateVertexArray()
{
	vertex_array_handle Handle;
	glCreateVertexArrays(1, &Handle.Idx);
	return Handle;
}

vertex_buffer_handle CreateVertexBuffer()
{
	vertex_buffer_handle Handle;
	glCreateBuffers(1, &Handle.Idx);
	return Handle;
}

index_buffer_handle CreateIndexBuffer()
{
	index_buffer_handle Handle;
	glCreateBuffers(1, &Handle.Idx);
	return Handle;
}

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
