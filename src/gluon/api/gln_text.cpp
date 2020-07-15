#include "gln_text.h"

#include <stb_image.h>

#include <rapidjson/document.h>

#include <loguru.hpp>

#include <stdio.h>

namespace gluon
{

font_atlas LoadFontAtlas(const char* FontName)
{
	font_atlas Atlas;

	// Assume FontName.png / FontName.json
	char FontAtlas[256] = {};
	char FontData[256]  = {};

	strcat(FontAtlas, "resources/fonts/");
	strcat(FontAtlas, FontName);
	strcat(FontAtlas, ".png");

	strcat(FontData, "resources/fonts/");
	strcat(FontData, FontName);
	strcat(FontData, ".json");

	// Load png
	i32 Channels;
	// stbi_set_flip_vertically_on_load(true);
	u8* Data = stbi_load(FontAtlas, &Atlas.Width, &Atlas.Height, &Channels, 3);

	if (Data == nullptr)
	{
		LOG_F(ERROR, "Cannot find or open %s", FontAtlas);
		return Atlas;
	}

	u32 TotalSize = Atlas.Width * Atlas.Height * Channels;
	Atlas.Data    = (u8*)malloc(TotalSize);
	memcpy(Atlas.Data, Data, TotalSize);

	stbi_image_free(Data);

	FILE* JsonFile = fopen(FontData, "r");

	if (JsonFile == nullptr)
	{
		LOG_F(ERROR, "Cannot find or open %s", FontData);
		return Atlas;
	}

	fseek(JsonFile, SEEK_SET, SEEK_END);
	u32 FileSize = ftell(JsonFile);
	rewind(JsonFile);

	char*  Buffer = (char*)malloc(FileSize + 1);
	size_t Read   = fread(Buffer, 1, FileSize, JsonFile);
	Buffer[Read]  = '\0';

	fclose(JsonFile);

	rapidjson::Document Document;
	Document.Parse(Buffer);

	if (Document.IsObject())
	{
		if (Document.HasMember("metrics"))
		{
			const auto& FontMetrics = Document["metrics"].GetObject();

			auto GetOrDefault = [&FontMetrics](const char* Name) {
				if (FontMetrics.HasMember(Name))
				{
					return FontMetrics[Name].GetFloat();
				}
				return 0.0f;
			};

			font_metrics Metrics;
			Metrics.LineHeight         = GetOrDefault("lineHeight");
			Metrics.Ascender           = GetOrDefault("ascender");
			Metrics.Descender          = GetOrDefault("descender");
			Metrics.UnderlineY         = GetOrDefault("underlineY");
			Metrics.UnderlineThickness = GetOrDefault("underlineThickness");
			Atlas.Metrics              = Metrics;
		}

		if (Document.HasMember("glyphs") && Document["glyphs"].IsArray())
		{
			const auto& Glyphs = Document["glyphs"].GetArray();

			for (const auto& Glyph : Glyphs)
			{
				if (!Glyph.HasMember("unicode") || !Glyph.HasMember("advance"))
				{
					continue;
				}

				glyph GlyphInfo;
				GlyphInfo.Unicode = Glyph["unicode"].GetUint();
				GlyphInfo.Advance = Glyph["advance"].GetFloat();

				if (Glyph.HasMember("planeBounds") && Glyph.HasMember("atlasBounds"))
				{
					GlyphInfo.HasGeometry        = true;
					GlyphInfo.AtlasBounds.Left   = Glyph["atlasBounds"]["left"].GetFloat();
					GlyphInfo.AtlasBounds.Bottom = Glyph["atlasBounds"]["bottom"].GetFloat();
					GlyphInfo.AtlasBounds.Right  = Glyph["atlasBounds"]["right"].GetFloat();
					GlyphInfo.AtlasBounds.Top    = Glyph["atlasBounds"]["top"].GetFloat();
					GlyphInfo.PlaneBounds.Left   = Glyph["planeBounds"]["left"].GetFloat();
					GlyphInfo.PlaneBounds.Bottom = Glyph["planeBounds"]["bottom"].GetFloat();
					GlyphInfo.PlaneBounds.Right  = Glyph["planeBounds"]["right"].GetFloat();
					GlyphInfo.PlaneBounds.Top    = Glyph["planeBounds"]["top"].GetFloat();
				}
				else
				{
					// Essentially <space> character.
					GlyphInfo.HasGeometry = false;
				}

				Atlas.Glyphs[GlyphInfo.Unicode] = GlyphInfo;

				// LOG_F(INFO, "Glyph: %d %c", Glyph["unicode"].GetUint(), Glyph["unicode"].GetUint());
			}
		}

		if (Document.HasMember("kerning") && Document["kerning"].IsArray())
		{
			const auto& Kernings = Document["kerning"].GetArray();

			for (const auto& Kerning : Kernings)
			{
				if (Kerning.HasMember("unicode1") && Kerning.HasMember("unicode2") && Kerning.HasMember("advance"))
				{
					const i32 Unicode1                      = Kerning["unicode1"].GetInt();
					const i32 Unicode2                      = Kerning["unicode2"].GetInt();
					const f32 Advance                       = Kerning["advance"].GetFloat();
					Atlas.GlyphKernings[Unicode1][Unicode2] = Advance;
				}
			}
		}
	}

	free(Buffer);

	return Atlas;
}
}
