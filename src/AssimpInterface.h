#pragma once
#include "Common.h"
#include "Texture.h"
#include "Mesh.h"
#include <map>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

class aiSceneWrapper
{
public:
	aiSceneWrapper();
	~aiSceneWrapper();

	std::vector<aiMesh*> mMeshes;
	std::vector<aiMaterial*> mMaterials;

	aiNode* mRootNode;
	std::vector<aiNode*> mAllNodes;

	aiScene* Finish();
	void AddNodeToRoot(aiNode* node);
};

class aiMeshWrapper
{
public:
	aiMeshWrapper();
	~aiMeshWrapper();

	std::vector<aiVector3D> mVertices;
	std::vector<aiColor4D> mColors;
	std::vector<aiFace> mFaces;
	std::vector<aiVector3D> mTextureCoords;
	unsigned int mMaterialIndex;
	std::string name;

	aiMesh* Finish();
};

struct ExportFormat
{
	std::string id;
	std::string desc;
	std::string ext;

	ExportFormat(const aiExportFormatDesc* desc);
};

class AssimpExporter
{
public:
	AssimpExporter();
	~AssimpExporter();

	void BeginExport();

	void AddTexture(std::string name, Texture* texture);

	void AddMesh(Mesh* mesh);

	void EndExport(std::string folderPath, std::string mapname, ExportFormat format);

	static std::vector<ExportFormat> GetExportOptions();

private:
	// Call AddMesh instead
	unsigned int AddSection(MeshSection* section, Mesh* parent);

	std::map<std::string, int> textureMap;
	std::map<int, std::string> textureUnmap;
	std::vector<Texture*> textureList;

	aiSceneWrapper* scene;
};