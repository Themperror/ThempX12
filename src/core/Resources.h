#pragma once

#include "renderer/pass.h"
#include "renderer/texture.h"
#include "renderer/types.h"

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
		std::vector<D3D::SubPass>& LoadMaterials();

		static std::unique_ptr<Resources> instance;


		D3D::Texture& Get(D3D::RTVHandle handle);
		D3D::Texture& Get(D3D::DSVHandle handle);
		D3D::Texture& Get(D3D::SRVHandle handle);
		D3D::SubPass& Get(D3D::SubPassHandle handle);
		D3D::Pass& Get(D3D::PassHandle handle);
	private:

		D3D::ShaderHandle LoadShader(std::string_view filename);
		D3D::RenderTargetHandle LoadRenderTarget(std::string_view filename);
		D3D::PassHandle LoadPass(std::string_view filename);
		std::vector<D3D::SubPass> LoadMaterial(const std::string& data);
		
		void AddOrSetShaderToPass(std::string_view name, int index, Themp::D3D::Pass& pass);

		void MergePasses(std::vector<D3D::SubPass> passes);


		std::vector<Themp::D3D::Pass> m_Passes;
		std::vector<std::pair<std::string,D3D::Texture>> m_DepthTargets;
		std::vector<std::pair<std::string,D3D::Texture>> m_ColorTargets;
		std::vector<std::pair<std::string,D3D::Texture>> m_SRVs;
		std::vector<D3D::SubPass> m_Subpasses;
	};
};