#include "game/game.h"

#include <lib/imgui/imgui.h>
#include "core/renderer/gpu_resources.h"
#include "core/renderer/control.h"
#include "core/engine.h"
#include "core/resources.h"
#include "core/scripting/asengine.h"

void Game::Game::Start()
{
	auto model = Themp::Engine::instance->m_Renderer->GetResourceManager().Test_GetAndAddRandomModel();

	Themp::D3D::Control& renderer = *Themp::Engine::instance->m_Renderer;
	//renderer.AddSubPass(mainHandle, 0);
	//renderer.Register(mainHandle, subHandle, model);
	//
	//auto subPass = m_MainPass.AddSubPass(0); // AddSubPass(model.GetShadowMaterial())
	//subPass.Register(model);
}

void Game::Game::Stop()
{

}

void Game::Game::Update(double delta)
{
	if (ImGui::Begin("Game"))
	{
		if (ImGui::Button("Add Object3D"))
		{
			Themp::D3D::Object3D obj;
			obj.m_Transform.SetPosition(0, 0, 30);
			obj.m_ScriptHandle = Themp::Engine::instance->m_Scripting->AddScript("quad");
			obj.m_Name = "newObj";
			Themp::Engine::instance->m_Scripting->LinkToObject3D(obj.m_ScriptHandle, obj.m_Name);
			Themp::Engine::instance->m_Resources->AddObject3D(obj);
		}
	}
	ImGui::End();
}
