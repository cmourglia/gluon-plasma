#include <gluon/render_backend/gln_renderbackend.h>

#include <glad/glad.h>
#include <EASTL/array.h>
#include <loguru.hpp>

#include <stdio.h>

namespace gln
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

void InitializeBackend()
{
	if (!gladLoadGL())
	{
		LOG_F(FATAL, "Cannot load OpenGL functions");
	}

	LOG_F(INFO, "OpenGL:\n\tVersion %s\n\tVendor %s", glGetString(GL_VERSION), glGetString(GL_VENDOR));
}

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

using data_type_array       = eastl::array<GLenum, DataType_Count>;
using format_array          = eastl::array<GLenum, 4>;
using internal_format_array = eastl::array<data_type_array, 4>;
using wrap_mode_array       = eastl::array<GLenum, WrapMode_Count>;
using min_filter_array      = eastl::array<GLenum, MinFilter_Count>;
using mag_filter_array      = eastl::array<GLenum, MagFilter_Count>;

constexpr auto GenerateDataTypeArray()
{
	data_type_array Result = {GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT};
	return Result;
}

constexpr auto GenerateFormatArray()
{
	format_array Result = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
	return Result;
}

constexpr auto GenerateInternalFormatArray()
{
	internal_format_array Result = {};

	Result[0] = {GL_R8, GL_R8, GL_R16, GL_R16, GL_R32I, GL_R32UI, GL_R32F};
	Result[1] = {GL_RG8, GL_RG8, GL_RG16, GL_RG16, GL_RG32I, GL_RG32UI, GL_RG32F};
	Result[2] = {GL_RGB8, GL_RGB8, GL_RGB16, GL_RGB16, GL_RGB32I, GL_RGB32UI, GL_RGB32F};
	Result[3] = {GL_RGBA8, GL_RGBA8, GL_RGBA16, GL_RGBA16, GL_RGBA32I, GL_RGBA32UI, GL_RGBA32F};

	return Result;
}

constexpr auto GenerateWrapModeArray()
{
	wrap_mode_array Result = {GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE};

	return Result;
}

constexpr auto GenerateMinFilterArray()
{
	min_filter_array Result =
	    {GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR};
	return Result;
}

constexpr auto GenerateMagFilterArray()
{
	mag_filter_array Result = {GL_NEAREST, GL_LINEAR};
	return Result;
}

constexpr data_type_array       k_DataTypesToGL       = GenerateDataTypeArray();
constexpr format_array          k_FormatsToGL         = GenerateFormatArray();
constexpr internal_format_array k_InternalFormatsToGL = GenerateInternalFormatArray();
constexpr wrap_mode_array       k_WrapModesToGL       = GenerateWrapModeArray();
constexpr min_filter_array      k_MinFiltersToGL      = GenerateMinFilterArray();
constexpr mag_filter_array      k_MagFiltersToGL      = GenerateMagFilterArray();

inline GLenum GetDataType(data_type DataType) { return k_DataTypesToGL[DataType]; }

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

inline GLenum GetFormat(u32 ComponentCount) { return k_FormatsToGL[ComponentCount - 1]; }
inline GLenum GetInternalFormat(u32 ComponentCount, data_type DataType) { return k_InternalFormatsToGL[ComponentCount - 1][DataType]; }
inline GLenum GetWrapMode(wrap_mode Mode) { return k_WrapModesToGL[Mode]; }
inline GLenum GetMinFilter(min_filter Filter) { return k_MinFiltersToGL[Filter]; }
inline GLenum GetMagFilter(mag_filter Filter) { return k_MagFiltersToGL[Filter]; }
}

#include "gln_renderbackend_program.cpp"
#include "gln_renderbackend_buffer.cpp"
#include "gln_renderbackend_texture.cpp"
