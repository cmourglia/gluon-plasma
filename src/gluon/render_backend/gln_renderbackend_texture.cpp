#include <gluon/render_backend/gln_renderbackend.h>

#include <gluon/core/gln_math.h>

#include <glad/glad.h>

#include <loguru.hpp>

#include <EASTL/unordered_map.h>
#include <EASTL/array.h>

#include <assert.h>

namespace gln
{

struct texture_info
{
	u32       Width, Height;
	u32       ComponentCount;
	data_type DataType;
	bool      WithMipmap;
};

static eastl::unordered_map<u32, texture_info> TextureInfos;

texture_handle CreateTexture(u32 Width, u32 Height, u32 ComponentCount, data_type DataType, bool WithMipmap, void* Data)
{
	texture_handle Texture;

	glCreateTextures(GL_TEXTURE_2D, 1, &Texture.Idx);

	u32 Levels = 1;
	if (WithMipmap)
	{
		Levels = (u32)(log2f(Min((f32)Width, (f32)Height)));
	}

	glTextureStorage2D(Texture.Idx, Levels, GetInternalFormat(ComponentCount, DataType), Width, Height);

	SetTextureFiltering(Texture, MinFilter_Linear, MagFilter_Linear);
	SetTextureWrapping(Texture, WrapMode_Repeat, WrapMode_Repeat);

	TextureInfos[Texture.Idx] = {Width, Height, ComponentCount, DataType, WithMipmap};

	if (Data != nullptr)
	{
		SetTextureData(Texture, Data);
	}

	return Texture;
}

void SetTextureData(texture_handle Handle, void* Data)
{
	const auto& Info = TextureInfos[Handle.Idx];
	glTextureSubImage2D(Handle.Idx, 0, 0, 0, Info.Width, Info.Height, GetFormat(Info.ComponentCount), GetDataType(Info.DataType), Data);

	if (Info.WithMipmap)
	{
		glGenerateTextureMipmap(Handle.Idx);
	}
}

void SetTextureWrapping(texture_handle Handle, wrap_mode WrapS, wrap_mode WrapT)
{
	glTextureParameteri(Handle.Idx, GL_TEXTURE_WRAP_S, GetWrapMode(WrapS));
	glTextureParameteri(Handle.Idx, GL_TEXTURE_WRAP_T, GetWrapMode(WrapT));
}

void SetTextureFiltering(texture_handle Handle, min_filter MinFilter, mag_filter MagFilter)
{
	glTextureParameteri(Handle.Idx, GL_TEXTURE_MIN_FILTER, GetMinFilter(MinFilter));
	glTextureParameteri(Handle.Idx, GL_TEXTURE_MAG_FILTER, GetMagFilter(MagFilter));
}

void DestroyTexture(texture_handle Handle) { glDeleteTextures(1, &Handle.Idx); }

}
