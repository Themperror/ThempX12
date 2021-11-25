#pragma once

#include "renderer/pass.h"

namespace Themp
{
	//struct Vertex;
	//struct Texture;
	//class Object3D;
	//class Mesh;
	//class Material;

	class Resources
	{
	public:
		void LoadMaterials();

		static std::unique_ptr<Resources> instance;


	private:

		D3D::ShaderHandle LoadShader(std::string_view filename);
		D3D::Pass LoadPass(std::string_view filename) const;
		std::vector<Themp::D3D::Pass> LoadMaterial(const std::string& data);
		
		void AddOrSetShaderToPass(std::string_view name, int index, Themp::D3D::Pass& pass);

		void MergePasses(std::vector<Themp::D3D::Pass> passes);
		//size_t currentUniqueMatIndex = 0;
		//Object3D* LoadModel(std::string name);
		std::vector<Themp::D3D::Pass> m_Passes;
	};
};