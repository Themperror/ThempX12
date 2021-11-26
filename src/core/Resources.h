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
		struct SubPass
		{
			Themp::D3D::PassHandle pass;
			Themp::D3D::ShaderHandle shader;
		};
	public:
		void LoadMaterials();

		static std::unique_ptr<Resources> instance;


	private:

		D3D::ShaderHandle LoadShader(std::string_view filename);
		D3D::RenderTargetHandle LoadRenderTarget(std::string_view filename);
		D3D::PassHandle LoadPass(std::string_view filename);
		std::vector<SubPass> LoadMaterial(const std::string& data);
		
		void AddOrSetShaderToPass(std::string_view name, int index, Themp::D3D::Pass& pass);

		void MergePasses(std::vector<SubPass> passes);
		//size_t currentUniqueMatIndex = 0;
		//Object3D* LoadModel(std::string name);
		std::vector<Themp::D3D::Pass> m_Passes;
		std::vector<SubPass> m_Subpasses;
	};
};