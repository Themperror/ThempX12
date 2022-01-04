#include "modelexport.h"


#include "core/util/fileutils.h"
#include "core/util/stringutils.h"

#include "core/renderer/types.h"

#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include <cstdio>


#include <lib/assimp/Importer.hpp>      // C++ importer interface
#include <lib/assimp/scene.h>           // Output data structure
#include <lib/assimp/postprocess.h>     // Post processing flags

namespace Themp
{
	FILETIME ModelExport::fileCreationTime, ModelExport::fileReadTime, ModelExport::fileWriteTime;
	bool ModelExport::ImportFile(const std::string& pFile)
	{

		std::string filename = Util::GetFileName(pFile);
		std::string outputFolderPath = "..\\";
		outputFolderPath.append(EXPORT_MODELS_PATH);
		outputFolderPath.append(filename).append("\\");
		std::string outputModelPath = outputFolderPath;
		outputModelPath.append(filename).append(".model");

		{
			HANDLE inFileHandle = CreateFileA(pFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetFileTime(inFileHandle, &fileCreationTime, &fileReadTime, &fileWriteTime);
			CloseHandle(inFileHandle);
		}

		FILE* output = nullptr;
		fopen_s(&output, outputModelPath.c_str(), "rb");

		ModelHeader header{};
		if (output != nullptr)
		{
			fread(&header, sizeof(ModelHeader), 1, output);
			fclose(output);
			if (header.origLastWrite.dwLowDateTime == fileWriteTime.dwLowDateTime && header.origLastWrite.dwHighDateTime == fileWriteTime.dwHighDateTime && header.version == MODEL_VERSION)
			{
				//file is up-to-date, no need to re-export
				return true;
			}
		}

		// Create an instance of the Importer class
		Assimp::Importer importer;

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
		const aiScene* scene = importer.ReadFile(pFile,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_FindInstances |
			aiProcess_GenNormals |
			aiProcess_ImproveCacheLocality |
			aiProcess_Triangulate |
			aiProcess_SortByPType);

		// If the import failed, report it
		if (scene == nullptr)
		{
			std::cout << importer.GetErrorString() << std::endl;
			system("pause");
			return false;
		}

		// Now we can access the file's contents.
		ExportFile(pFile, scene);

		// We're done. Everything will be cleaned up by the importer destructor
		return true;
	}



	void ModelExport::HandleChilds(FILE* output, aiNode* node)
	{
		fwrite(&node->mNumMeshes, sizeof(unsigned int), 1, output);
		fwrite(&node->mNumChildren, sizeof(unsigned int), 1, output);

		uint32_t nameLength = node->mName.length;
		fwrite(&nameLength, sizeof(uint32_t), 1, output);
		if(nameLength != 0)
		{ 
			fwrite(node->mName.C_Str(), nameLength, 1, output);
		}
		fwrite(&node->mTransformation, sizeof(aiMatrix4x4), 1, output);
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			fwrite(&node->mMeshes[i], sizeof(unsigned int), 1, output);
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			HandleChilds(output, node->mChildren[i]);
		}
	}

	bool ModelExport::ExportFile(const std::string& inFilePath, const aiScene* scene)
	{
		std::string filename = Util::GetFileName(inFilePath);
		std::string outputFolderPath = "..\\";
		outputFolderPath.append(EXPORT_BASE_PATH);

		std::string outputModelPath = outputFolderPath;
		outputModelPath.append(EXPORT_MODELS_PATH);
		CreateDirectoryA(outputModelPath.c_str(), nullptr);
		outputModelPath.append(filename).append("\\");
		CreateDirectoryA(outputModelPath.c_str(), nullptr);
		outputModelPath.append(filename).append(".model");

		std::string outputMaterialPath = outputFolderPath;
		outputMaterialPath.append(EXPORT_MATERIALS_PATH);
		CreateDirectoryA(outputMaterialPath.c_str(), nullptr);
		outputMaterialPath.append(filename).append("\\");
		CreateDirectoryA(outputMaterialPath.c_str(), nullptr);

		FILE* output = nullptr;
		ModelHeader header{};

		fopen_s(&output, outputModelPath.c_str(), "wb");

		memcpy(header.magic, HEADERMAGIC, sizeof(HEADERMAGIC));
		header.origLastWrite = fileWriteTime;
		header.numMeshes = scene->mNumMeshes;
		header.numMaterials = scene->mNumMaterials;
		fwrite(&header, sizeof(ModelHeader), 1, output);

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			MeshHeader mesh;
			memset(&mesh, 0, sizeof(MeshHeader));

			mesh.numVertices = scene->mMeshes[i]->mNumVertices;
			std::vector<D3D::Vertex> vertices(mesh.numVertices);

			mesh.numIndices = scene->mMeshes[i]->mNumFaces * 3;

			std::vector<uint32_t> indices(mesh.numIndices);

			mesh.materialID = scene->mMeshes[i]->mMaterialIndex;
			for (size_t j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
			{
				vertices[j].position.x = scene->mMeshes[i]->mVertices[j].x;
				vertices[j].position.y = scene->mMeshes[i]->mVertices[j].y;
				vertices[j].position.z = scene->mMeshes[i]->mVertices[j].z;
				if (scene->mMeshes[i]->HasTextureCoords(0))
				{
					vertices[j].uv.x = scene->mMeshes[i]->mTextureCoords[0][j].x;
					vertices[j].uv.y = scene->mMeshes[i]->mTextureCoords[0][j].y;
				}
				else
				{
					vertices[j].uv.x = 1.0f;
					vertices[j].uv.y = 1.0f;
				}
				if (scene->mMeshes[i]->HasNormals())
				{
					vertices[j].normal.x = scene->mMeshes[i]->mNormals[j].x;
					vertices[j].normal.y = scene->mMeshes[i]->mNormals[j].y;
					vertices[j].normal.z = scene->mMeshes[i]->mNormals[j].z;
				}
				else
				{
					vertices[j].normal.x = 1.0f;
					vertices[j].normal.y = 0.0f;
					vertices[j].normal.z = 0.0f;
				}
				if (scene->mMeshes[i]->HasTangentsAndBitangents())
				{
					vertices[j].tangent.x = scene->mMeshes[i]->mTangents[j].x;
					vertices[j].tangent.y = scene->mMeshes[i]->mTangents[j].y;
					vertices[j].tangent.z = scene->mMeshes[i]->mTangents[j].z;
					vertices[j].bitangent.x = scene->mMeshes[i]->mBitangents[j].x;
					vertices[j].bitangent.y = scene->mMeshes[i]->mBitangents[j].y;
					vertices[j].bitangent.z = scene->mMeshes[i]->mBitangents[j].z;
				}
				else
				{
					vertices[j].tangent.x = 0;
					vertices[j].tangent.y = 1.0f;
					vertices[j].tangent.z = 0;
					vertices[j].bitangent.x = 0;
					vertices[j].bitangent.y = 0;
					vertices[j].bitangent.z = 1.0f;
				}
			}
			for (size_t j = 0; j < scene->mMeshes[i]->mNumFaces; j++)
			{
				indices[j * 3 + 0] = scene->mMeshes[i]->mFaces[j].mIndices[0];
				indices[j * 3 + 1] = scene->mMeshes[i]->mFaces[j].mIndices[1];
				indices[j * 3 + 2] = scene->mMeshes[i]->mFaces[j].mIndices[2];
			}

			fwrite(&mesh, sizeof(MeshHeader), 1, output);
			fwrite(vertices.data(), sizeof(D3D::Vertex) * vertices.size(), 1, output);
			fwrite(indices.data(), sizeof(uint32_t) * indices.size(), 1, output);

		}
		for (size_t i = 0; i < header.numMaterials; i++)
		{
			uint32_t numTextures = 0;
			aiString matName;
			scene->mMaterials[i]->Get(AI_MATKEY_NAME, matName);
			std::string matNameString = matName.C_Str();
			if (matNameString.size() < 1)
			{
				matNameString = "default";
			}

			std::string materialFile = outputMaterialPath + matNameString + ".mat";

	
			for (size_t j = 0; j < AI_TEXTURE_TYPE_MAX; j++)
			{
				int texturecount = scene->mMaterials[i]->GetTextureCount((aiTextureType)j);
				numTextures += texturecount;
			}
			std::vector<uint8_t> textureTypes;
			textureTypes.resize(numTextures);

			uint32_t stringSize = static_cast<uint32_t>(matNameString.size());
			
			fwrite(&stringSize, sizeof(uint32_t), 1, output);
			fwrite(matNameString.c_str(), matNameString.size(), 1, output);
			int currentTextureIndex = 0;
			std::vector<std::pair<uint8_t, std::string>> textures;

			if (numTextures == 0)
				goto CREATE_MATERIAL_IF_NEEDED;

			for (uint8_t j = 0; j < AI_TEXTURE_TYPE_MAX; j++)
			{
				int texturecount = scene->mMaterials[i]->GetTextureCount((aiTextureType)j);
				if (texturecount > 1)
				{
					Themp::Print("Found model [%s] with more than 1 texture with aiTextureType: %i", filename.c_str(), j);
				}

				for (int32_t k = 0; k < texturecount; k++)
				{
					aiTextureType t = (aiTextureType)j;
					textureTypes[currentTextureIndex] = j;
					aiString texturePath;
					aiGetMaterialTexture(scene->mMaterials[i], (aiTextureType)j, k, &texturePath);
					std::string str = Util::ReplaceChar(texturePath.C_Str(),'\\', '/');
					//str = Util::ReplaceExtensionWith(str,".dds");
					uint32_t strSize = static_cast<uint32_t>(str.size());

					textures.push_back({ t, str });
					currentTextureIndex++;
				}
			}

			CREATE_MATERIAL_IF_NEEDED:
			FILE* f = nullptr;
			fopen_s(&f, materialFile.c_str(), "r");
			if (f == nullptr)
			{
				std::ofstream matFile(materialFile);
				if (matFile.good())
				{
					matFile << "## ThempX12 Material File ##\n\n";
					matFile << "[[SubPass]]\n";
					matFile << "PositionInfo = true\n";
					matFile << "UVInfo = true\n";
					matFile << "NormalInfo = true\n";
					matFile << "Pass = \"default\"\n";
					matFile << "Shader = \"default\"\n";
					matFile << "Textures = [\n";
					for (int l = 0; l < textures.size(); l++)
					{
						matFile << "[" << (int)textures[l].first << ", \"" << textures[l].second << "\"],\n";
					}
					matFile << "]\n";
					matFile.close();
				}
			}
			else
			{
				fclose(f);
			}

		}


		auto root = scene->mRootNode;
		HandleChilds(output, root);
		fclose(output);
		return true;
	}
}
