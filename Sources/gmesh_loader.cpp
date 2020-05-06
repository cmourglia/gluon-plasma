#include "gmesh_loader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <loguru.hpp>

static void ProcessMesh(aiMesh* SrcMesh, const aiScene* Scene, GModel* Model)
{
	Model->Vertices.reserve(Model->Vertices.size() + SrcMesh->mNumVertices);

	const bool HasTexcoords = (SrcMesh->mTextureCoords[0] != nullptr);

	const u32 BaseVertex = (u32)Model->Vertices.size();

	for (u32 Index = 0; Index < SrcMesh->mNumVertices; ++Index)
	{
		GVertex Vertex;

		Vertex.Position = {SrcMesh->mVertices[Index].x, SrcMesh->mVertices[Index].y, SrcMesh->mVertices[Index].z};
		Vertex.Normal   = {SrcMesh->mNormals[Index].x, SrcMesh->mNormals[Index].y, SrcMesh->mNormals[Index].z};

		if (HasTexcoords)
		{
			Vertex.Texcoord = {SrcMesh->mTextureCoords[0][Index].x, SrcMesh->mTextureCoords[0][Index].y};
		}
		else
		{
			Vertex.Texcoord = {0.0f, 0.0f};
		}

		Model->Vertices.push_back(Vertex);
	}

	const u32 BaseIndex = (u32)Model->Indices.size();

	for (u32 FaceIndex = 0; FaceIndex < SrcMesh->mNumFaces; ++FaceIndex)
	{
		const aiFace Face = SrcMesh->mFaces[FaceIndex];
		for (u32 Index = 0; Index < Face.mNumIndices; ++Index)
		{
			Model->Indices.push_back(Face.mIndices[Index]);
		}
	}

	const u32 IndexCount = (u32)Model->Indices.size() - BaseIndex;

	GMesh Mesh;
	Mesh.BaseVertex = BaseVertex;
	Mesh.BaseIndex  = BaseIndex;
	Mesh.IndexCount = IndexCount;

	Model->Meshes.push_back(Mesh);
}

static void ProcessNode(aiNode* Node, const aiScene* Scene, GModel* Model, glm::mat4 CurrentTransform)
{
	// clang-format off
	const glm::mat4 NodeTransform(Node->mTransformation.a1, Node->mTransformation.b1, Node->mTransformation.c1, Node->mTransformation.d1,
	                              Node->mTransformation.a2, Node->mTransformation.b2, Node->mTransformation.c2, Node->mTransformation.d2,
	                              Node->mTransformation.a3, Node->mTransformation.b3, Node->mTransformation.c3, Node->mTransformation.d3,
	                              Node->mTransformation.a4, Node->mTransformation.b4, Node->mTransformation.c4, Node->mTransformation.d4);
	// clang-format on

	// FIXME: I think I will never remember multiplication order
	const glm::mat4 Transform = NodeTransform * CurrentTransform;

	for (u32 Index = 0; Index < Node->mNumMeshes; ++Index)
	{
		ProcessMesh(Scene->mMeshes[Node->mMeshes[Index]], Scene, Model);
		Model->Meshes.back().ModelMatrix = Transform;
	}

	for (u32 Index = 0; Index < Node->mNumChildren; ++Index)
	{
		ProcessNode(Node->mChildren[Index], Scene, Model, Transform);
	}
}

bool LoadMesh(const char* Filename, GModel* Model)
{
	Assimp::Importer Importer;

	const u32      ImporterFlags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes;
	const aiScene* Scene         = Importer.ReadFile(Filename, ImporterFlags);

	if ((nullptr == Scene) || (0 != (Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) || (nullptr == Scene->mRootNode))
	{
		LOG_F(ERROR, "Assimp import error: %s", Importer.GetErrorString());
		return false;
	}

	ProcessNode(Scene->mRootNode, Scene, Model, glm::mat4(1.0f));

	return true;
}
