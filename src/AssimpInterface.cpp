#include "AssimpInterface.h"

#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "Utils/stb_image_write.h"

#include <filesystem>

aiSceneWrapper::aiSceneWrapper()
	: mMeshes(), mMaterials(), mAllNodes(), mRootNode(nullptr)
{ }

aiSceneWrapper::~aiSceneWrapper()
{
	mRootNode = nullptr;
	for (auto& itr : mAllNodes)
	{
		delete itr;
		itr = nullptr;
	}
	mAllNodes.clear();
	for (auto& itr : mMeshes)
	{
		delete itr;
		itr = nullptr;
	}
	mMeshes.clear();
	for (auto& itr : mMaterials)
	{
		delete itr;
		itr = nullptr;
	}
	mMaterials.clear();
}

aiScene* aiSceneWrapper::Finish()
{
	aiScene* scene = new aiScene();
	scene->mRootNode = mRootNode;
	scene->mNumMeshes = mMeshes.size();
	scene->mNumMaterials = mMaterials.size();

	aiMesh** meshArr = new aiMesh * [scene->mNumMeshes];
	for (int i = 0; i < scene->mNumMeshes; i++) { meshArr[i] = mMeshes[i]; }
	scene->mMeshes = meshArr;

	aiMaterial** matArr = new aiMaterial * [scene->mNumMaterials];
	for (int i = 0; i < scene->mNumMaterials; i++) { matArr[i] = mMaterials[i]; }
	scene->mMaterials = matArr;

	// These belong to the actual aiScene now.
	mMeshes.clear();
	mMaterials.clear();
	mAllNodes.clear();

	return scene;
}

aiMeshWrapper::aiMeshWrapper()
	: mVertices(), mColors(), mFaces(), mTextureCoords()
{ 
	mMaterialIndex = 0;
}

aiMeshWrapper::~aiMeshWrapper()
{
	// None of the vectors hold pointers so everything will clean up nicely.
	mVertices.clear();
	mColors.clear();
	mFaces.clear();
	mTextureCoords.clear();
}

aiMesh* aiMeshWrapper::Finish()
{
	aiMesh* mesh = new aiMesh();
	mesh->mPrimitiveTypes = aiPrimitiveType_POLYGON;
	mesh->mMaterialIndex = mMaterialIndex;

	mesh->mNumVertices = mVertices.size();
	mesh->mVertices = new aiVector3D[mesh->mNumVertices];
	for (int i = 0; i < mesh->mNumVertices; i++) { mesh->mVertices[i] = mVertices[i]; }

	mesh->mNumFaces = mFaces.size();
	mesh->mFaces = new aiFace[mesh->mNumFaces];
	for (int i = 0; i < mesh->mNumFaces; i++) { mesh->mFaces[i] = mFaces[i]; }

	mesh->mColors[0] = new aiColor4D[mesh->mNumVertices];
	for (int i = 0; i < mesh->mNumVertices; i++) { mesh->mColors[0][i] = mColors[i]; }

	mesh->mNumUVComponents[0] = 2;
	mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
	for (int i = 0; i < mesh->mNumVertices; i++) { mesh->mTextureCoords[0][i] = mTextureCoords[i]; }

	return mesh;
}

void aiSceneWrapper::AddNodeToRoot(aiNode* node)
{
	mAllNodes.push_back(node);
	mRootNode->addChildren(1, &node);
}

AssimpExporter::AssimpExporter()
{
	uint8 pix[] = { 0xFF, 0xFF, 0xFF, 0xFF }; 
	CTexture* dummyTexture = new CTexture(1, 1, pix, PF_RGBA32);
	AddTexture("dummyTexture", dummyTexture);
}

AssimpExporter::~AssimpExporter()
{
	if (scene)
	{
		delete scene;
		scene = nullptr;
	}
}

void AssimpExporter::BeginExport()
{
	scene = new aiSceneWrapper();
	aiNode* root = new aiNode();
	root->mTransformation = aiMatrix4x4();
	scene->mAllNodes.push_back(root);
	scene->mRootNode = root;
}

void AssimpExporter::AddTexture(std::string name, CTexture* texture)
{
	if (textureMap.find(name) != textureMap.end()) return;
	
	int idx = textureList.size();
	textureList.push_back(texture);
	textureMap.insert(std::make_pair(name, idx));
	textureUnmap.insert(std::make_pair(idx, name));
}

void AssimpExporter::AddMesh(Mesh* mesh)
{
	aiNode* node = new aiNode();

	/*glm::quat rot = glm::quat(mesh->rotation);
	aiVector3D position = aiVector3D(mesh->position.x, mesh->position.y, mesh->position.z);
	aiQuaternion rotation = aiQuaternion(mesh->rotation.x, mesh->rotation.y, mesh->rotation.z);
	aiVector3D scale = aiVector3D(mesh->scale.x, mesh->scale.y, mesh->scale.z);*/
	glm::quat rot = glm::quat(mesh->rotation);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, mesh->position);
	model = glm::rotate(model, glm::angle(rot), glm::axis(rot));
	model = glm::scale(model, mesh->scale);
	aiMatrix4x4 aiMatrix = aiMatrix4x4();
	aiMatrix[0][0] = model[0][0];	aiMatrix[1][0] = model[1][0];	aiMatrix[2][0] = model[2][0];	aiMatrix[3][0] = model[3][0];
	aiMatrix[0][1] = model[0][1];	aiMatrix[1][1] = model[1][1];	aiMatrix[2][1] = model[2][1];	aiMatrix[3][1] = model[3][1];
	aiMatrix[0][2] = model[0][2];	aiMatrix[1][2] = model[1][2];	aiMatrix[2][2] = model[2][2];	aiMatrix[3][2] = model[3][2];
	aiMatrix[0][3] = model[0][3];	aiMatrix[1][3] = model[1][3];	aiMatrix[2][3] = model[2][3];	aiMatrix[3][3] = model[3][3];
	aiMatrix.Transpose();

	//node->mTransformation = aiMatrix4x4(scale, rotation, position);
	node->mTransformation = aiMatrix;
	
	node->mNumMeshes = mesh->sections.size();
	node->mMeshes = new unsigned int[node->mNumMeshes];
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		node->mMeshes[i] = AddSection(mesh->sections[i], mesh);
	}

	scene->AddNodeToRoot(node);
}

unsigned int AssimpExporter::AddSection(MeshSection* section, Mesh* parent)
{
	aiMeshWrapper mesh;

	uint8 local_tex_id = section->raw->textureID;
	if (local_tex_id != 0xFF)
	{
		std::string texture_name = parent->texture_list[section->raw->textureID];
		mesh.mMaterialIndex = textureMap.at(texture_name);
	}
	else
	{
		mesh.mMaterialIndex = 0;
	}

	int fptr = 0;
	int fcount;
	if (section->draw_primative == GL_TRIANGLES)
	{
		for (int vert = 0; vert < section->vert_count; vert++)
		{
			fptr = vert * 10;
			float x, y, z, w, r, g, b, a, u, v;
			x = section->vert_data[fptr++];	y = section->vert_data[fptr++];	z = section->vert_data[fptr++];	w = section->vert_data[fptr++];
			r = section->vert_data[fptr++];	g = section->vert_data[fptr++];	b = section->vert_data[fptr++];	a = section->vert_data[fptr++];
			u = section->vert_data[fptr++];	v = section->vert_data[fptr++];
			mesh.mVertices.push_back(aiVector3D(x, y, z));
			mesh.mColors.push_back(aiColor4D(r, g, b, a));
			mesh.mTextureCoords.push_back(aiVector3D(u, v, 0.0f));
		}
		fcount = section->vert_count / 3;
		for (int f = 0; f < fcount; f++)
		{
			aiFace face;
			face.mNumIndices = 3;
			face.mIndices = new unsigned int[3];
			face.mIndices[0] = (f * 3) + 0;
			face.mIndices[1] = (f * 3) + 1;
			face.mIndices[2] = (f * 3) + 2;
			mesh.mFaces.push_back(face);
		}
	}
	else
	{
		assert(section->draw_primative == GL_TRIANGLE_STRIP);
		for (int vert = 0; vert < section->vert_count - 2; vert++)
		{
			for (int svert = 0; svert < 3; svert++)
			{
				int fptr = (vert+svert) * 10;
				float x, y, z, w, r, g, b, a, u, v;
				x = section->vert_data[fptr++];	y = section->vert_data[fptr++];	z = section->vert_data[fptr++];	w = section->vert_data[fptr++];
				r = section->vert_data[fptr++];	g = section->vert_data[fptr++];	b = section->vert_data[fptr++];	a = section->vert_data[fptr++];
				u = section->vert_data[fptr++];	v = section->vert_data[fptr++];
				mesh.mVertices.push_back(aiVector3D(x, y, z));
				mesh.mColors.push_back(aiColor4D(r, g, b, a));
				mesh.mTextureCoords.push_back(aiVector3D(u, v, 0.0f));
			}
		}
		fcount = section->vert_count - 2;
		for (int f = 0; f < fcount; f++)
		{
			aiFace face;
			face.mNumIndices = 3;
			face.mIndices = new unsigned int[3];
			if (f % 2 == 0)
			{
				face.mIndices[0] = (f * 3) + 0;
				face.mIndices[1] = (f * 3) + 1;
				face.mIndices[2] = (f * 3) + 2;
			}
			else
			{
				face.mIndices[0] = (f * 3) + 1;
				face.mIndices[1] = (f * 3) + 0;
				face.mIndices[2] = (f * 3) + 2;
			}
			mesh.mFaces.push_back(face);
		}
	}

	

	unsigned int res = scene->mMeshes.size();
	scene->mMeshes.push_back(mesh.Finish());
	return res;
}

void AssimpExporter::EndExport(std::string folderPath, std::string mapname)
{
	std::filesystem::path exportFolder = std::filesystem::path(folderPath);

	if (std::filesystem::exists(exportFolder))
	{
		if (!std::filesystem::is_directory(exportFolder))
		{
			// TODO: Error
			std::cerr << "[Export] Export path " << folderPath << " is not a directory!" << std::endl;
			return;
		}
	}
	else
	{
		if (!std::filesystem::create_directories(exportFolder))
		{
			std::cerr << "[Export] Could not create directory " << folderPath << "!" << std::endl;
			return;
		}
	}

	for (int i = 0; i < textureList.size(); i++)
	{
		std::string name = textureUnmap.at(i);

		if (name == "dummyTexture")
		{
			aiMaterial* mat = new aiMaterial();
			aiString n = aiString("dummyMaterial");
			aiColor4D c = aiColor4D(1, 1, 1, 1);
			mat->AddProperty(&n, AI_MATKEY_NAME);
			mat->AddProperty(&c, 1, AI_MATKEY_COLOR_DIFFUSE);
			scene->mMaterials.push_back(mat);
			continue;
		}

		std::filesystem::path outPath = exportFolder;
		outPath.append(name);
		outPath.replace_extension(".png");
		aiString filename = aiString(outPath.filename().string());
		aiString aiName = aiString(name);
		
		// Step 1: Build material
		aiMaterial* mat = new aiMaterial();
		mat->AddProperty(&aiName, AI_MATKEY_NAME);
		mat->AddProperty(&filename, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));
		scene->mMaterials.push_back(mat);
		
		// Step 2: Write out texture
		CTexture* tex = textureList[i];
		if (stbi_write_png(outPath.string().c_str(), tex->getWidth(), tex->getHeight(), 4, tex->getPixels(), tex->getWidth() * 4) == 0)
		{
			// TODO: error
			std::cerr << "[Export] Failed to write texture " << name << std::endl;
		}

		
	}

	// Step 3: Finalize scene
	aiScene* finalScene = scene->Finish();

	// Step 4: Export scene
	std::filesystem::path outFile = exportFolder;
	outFile.append(mapname);
	outFile.replace_extension(".fbx");

	Assimp::Exporter exp = Assimp::Exporter();
	if (AI_SUCCESS != exp.Export(finalScene, "fbx", outFile.string(), aiPostProcessSteps::aiProcess_FlipUVs))
	{
		std::cerr << "[Export] Failed to export fbx file!" << std::endl;
	}

	delete finalScene;
}