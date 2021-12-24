#pragma once

#include "renderer/pass.h"
#include "renderer/texture.h"
#include "renderer/types.h"
#include "renderer/shader.h"

namespace Themp
{
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
		D3D::Shader& Get(D3D::ShaderHandle handle);

		std::vector<std::pair<std::string, std::string>> GetScriptFiles();

	private:

		D3D::ShaderHandle LoadShader(std::string_view filename, std::vector<std::wstring>& shaderFiles);
		D3D::RenderTargetHandle LoadRenderTarget(std::string_view filename);
		D3D::PassHandle LoadPass(std::string_view filename);
		std::vector<D3D::SubPass> LoadMaterial(const std::string& data, std::vector<std::wstring>& shaderFiles);
		
		void AddOrSetShaderToPass(std::string_view name, int index, Themp::D3D::Pass& pass);

		void MergePasses(std::vector<D3D::SubPass> passes);


		std::vector<Themp::D3D::Pass> m_Passes;
		std::vector<std::pair<std::string,D3D::Texture>> m_DepthTargets;
		std::vector<std::pair<std::string,D3D::Texture>> m_ColorTargets;
		std::vector<std::pair<std::string,D3D::Texture>> m_SRVs;
		std::vector<D3D::SubPass> m_Subpasses;
		std::vector<D3D::Shader> m_Shaders;
	};
};