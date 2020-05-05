#pragma once

#include "defines.h"

#include <glm/glm.hpp>

#include <vector>

struct GVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 Texcoord;
};

struct GMesh
{
	u32 BaseIndex;
	u32 IndexCount;

	glm::mat4 ModelMatrix;
};

struct GModel
{
	std::vector<GVertex> Vertices;
	std::vector<u32>     Indices;

	std::vector<GMesh> Meshes;
};

bool LoadMesh(const char* Filename, GModel* Model);
