#pragma once
#include "core/engine.h"

#define EXPORT_BASE_PATH "resources\\"
#define EXPORT_MODELS_PATH "models\\"
#define EXPORT_MATERIALS_PATH "materials\\"
#define MODEL_VERSION 1
#define HEADERMAGIC "ThempXModel"

//Assimp types
struct aiNode;
struct aiScene;

namespace Themp
{
	class ModelExport
	{
	public:
		struct ModelHeader
		{
			char magic[12] = HEADERMAGIC;
			unsigned int version = 1;
			FILETIME origLastWrite;
			size_t numMeshes;
			size_t numMaterials;
		};

		struct MeshHeader
		{
			uint32_t numVertices;
			uint32_t numIndices;
			uint32_t materialID;
		};

		static uint32_t Version;
		static char HeaderMagic[12];
		static bool ImportFile(const std::string& pFile);
	private:
		static bool ExportFile(const std::string& inFilePath, const aiScene* scene);
		static void HandleChilds(FILE* output, aiNode* node);

		static FILETIME fileCreationTime, fileReadTime, fileWriteTime;
	};
}