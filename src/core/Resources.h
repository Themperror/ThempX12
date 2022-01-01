#pragma once

#include "core/renderer/pass.h"
#include "core/renderer/texture.h"
#include "core/renderer/types.h"
#include "core/renderer/shader.h"
#include "core/renderer/material.h"
#include "core/components/sceneobject.h"
#include "core/renderer/model.h"


namespace Themp
{
	namespace D3D
	{
		class Control;
	}

	class Resources
	{
	public:

		struct ResourceTextureInfo
		{
			std::string name;
			float scalar;
			bool doesScale;
			bool createSRV;
			D3D12_CLEAR_VALUE clearValue;
		};

		static std::unique_ptr<Resources> instance;

		Themp::D3D::Model LoadModel(D3D::Control& control, std::string modelName);

		void ResizeRendertargets(int width, int height);
		void LoadMaterials();
		D3D::Texture& Get(D3D::RTVHandle handle);
		D3D::Texture& Get(D3D::DSVHandle handle);
		D3D::Texture& Get(D3D::SRVHandle handle);
		D3D::SubPass& Get(D3D::SubPassHandle handle);
		D3D::Pass& Get(D3D::PassHandle handle);
		D3D::Shader& Get(D3D::ShaderHandle handle);
		D3D::Material& Get(D3D::MaterialHandle handle);

		std::vector<std::pair<ResourceTextureInfo, D3D::Texture>>& GetAllDSVs() { return m_DepthTargets; }
		size_t GetAmountOfSubpasses() const { return m_Subpasses.size(); };
		std::vector<std::pair<std::string, std::string>> GetScriptFiles() const;

		void LoadScene(std::string sceneFile);
		void AddSceneObject(SceneObject obj);
		Themp::D3D::Model LoadModel(std::string name);

		const std::vector<Themp::SceneObject>& GetSceneObjects() const;
		std::vector<Themp::SceneObject>& GetSceneObjects();

		static std::string_view GetResourcesFolder();
		static std::string_view GetMaterialsFolder();
		static std::string_view GetModelsFolder();
		static std::string_view GetScenesFolder();
		static std::string_view GetScriptsFolder();
		static std::string_view GetPassesFolder();
		static std::string_view GetShadersFolder();
		static std::string_view GetRenderTargetsFolder();
	   
	private:
		D3D::ShaderHandle LoadShader(std::string_view filename, std::vector<std::string>& shaderFiles);
		D3D::RenderTargetHandle LoadRenderTarget(std::string_view filename);
		D3D::PassHandle LoadPass(std::string_view filename);
		std::vector<D3D::SubPass> LoadMaterial(const std::string& data, std::vector<std::string>& shaderFiles);
		
		void AddOrSetShaderToPass(std::string_view name, int index, Themp::D3D::Pass& pass);

		void MergePasses(std::vector<D3D::SubPass> passes);

		std::vector<Themp::D3D::Pass> m_Passes;
		std::vector<std::pair<ResourceTextureInfo,D3D::Texture>> m_DepthTargets;
		std::vector<std::pair<ResourceTextureInfo, D3D::Texture>> m_ColorTargets = { { {"*swapchain",1.0f,true, false,{} }, {}} };
		std::vector<std::pair<ResourceTextureInfo,D3D::Texture>> m_SRVs;
		std::vector<D3D::SubPass> m_Subpasses;
		std::vector<D3D::Shader> m_Shaders;
		std::vector<D3D::Material> m_Materials;

		std::vector<D3D::Model> m_Models;

		std::vector<SceneObject> m_SceneObjects;
	};
};