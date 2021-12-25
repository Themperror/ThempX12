#pragma once

#include "renderer/pass.h"
#include "renderer/texture.h"
#include "renderer/types.h"
#include "renderer/shader.h"
#include "renderer/material.h"
#include "renderer/object3d.h"
#include "renderer/model.h"


namespace Themp
{
	namespace D3D
	{
		class Control;
	}

	class Resources
	{
	public:
		static std::unique_ptr<Resources> instance;

		Themp::D3D::Model LoadModel(D3D::Control& control, std::string modelName);

		void LoadMaterials();
		D3D::Texture& Get(D3D::RTVHandle handle);
		D3D::Texture& Get(D3D::DSVHandle handle);
		D3D::Texture& Get(D3D::SRVHandle handle);
		D3D::SubPass& Get(D3D::SubPassHandle handle);
		D3D::Pass& Get(D3D::PassHandle handle);
		D3D::Shader& Get(D3D::ShaderHandle handle);

		size_t GetAmountOfSubpasses() const { return m_Subpasses.size(); };
		std::vector<std::pair<std::string, std::string>> GetScriptFiles();

		void LoadScene(std::string sceneFile);
		void AddObject3D(D3D::Object3D obj);

		const std::vector<Themp::D3D::Object3D>& GetSceneObjects();

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
		std::vector<std::pair<std::string,D3D::Texture>> m_DepthTargets;
		std::vector<std::pair<std::string,D3D::Texture>> m_ColorTargets;
		std::vector<std::pair<std::string,D3D::Texture>> m_SRVs;
		std::vector<D3D::SubPass> m_Subpasses;
		std::vector<D3D::Shader> m_Shaders;
		std::vector<D3D::Material> m_Materials;

		std::vector<D3D::Model> m_Models;

		std::vector<D3D::Object3D> m_3DObjects;
	};
};