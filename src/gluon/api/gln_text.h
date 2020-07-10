#pragma once

#include <gluon/core/gln_defines.h>

#include <EASTL/unordered_map.h>

namespace gln
{

struct glyph
{
	u32 Unicode;
	f32 Advance;

	struct
	{
		f32 Left, Right, Top, Bottom;
	} PlaneBounds;

	struct
	{
		f32 Left, Right, Top, Bottom;
	} AtlasBounds;

	bool HasGeometry;
};

struct font_metrics
{
	f32 LineHeight         = 0.0f;
	f32 Ascender           = 0.0f;
	f32 Descender          = 0.0f;
	f32 UnderlineY         = 0.0f;
	f32 UnderlineThickness = 0.0f;
};

using glyph_kernings = eastl::unordered_map<u32, eastl::unordered_map<u32, f32>>;
using glyph_infos    = eastl::unordered_map<u32, glyph>;

struct font_atlas
{
	i32 Width = -1, Height = -1;
	u8* Data = nullptr;

	font_metrics Metrics;

	glyph_infos    Glyphs;
	glyph_kernings GlyphKernings;
};

// Assumes font to be in resources/fonts path.
font_atlas LoadFontAtlas(const char* FontName);
}
