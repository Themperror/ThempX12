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

float totalTime = 0.0f;
void Game::Game::Update(double delta)
{
	auto& sceneObjs = Themp::Engine::instance->m_Resources->GetSceneObjects();
	totalTime += delta;
	int i = 0;
	float tau = 3.1415f * 2.0f;
	float circleSlice = (tau / (float)sceneObjs.size());

	if (ImGui::Begin("Game"))
	{
		if (ImGui::Button("Add SceneObject"))
		{
			Themp::SceneObject obj;
			obj.m_Transform.SetPosition(0, 0, 30);
			obj.m_ScriptHandle = Themp::Engine::instance->m_Scripting->AddScript("quad");
			obj.m_Name = "newObj";
			Themp::Engine::instance->m_Scripting->LinkToSceneObject(obj.m_ScriptHandle, obj.m_Name);
			Themp::Engine::instance->m_Resources->AddSceneObject(obj);
		}
		ImGui::Text("delta value: %f", delta);
		ImGui::Text("totalTime value: %f", totalTime);
		ImGui::Text("sin value: %f", sin(circleSlice + totalTime) / 3.0f);
		ImGui::Text("cos value: %f", cos(circleSlice + totalTime) / 3.0f);
	}
	ImGui::End();

	for (auto& obj : sceneObjs)
	{
		float currentIndex = i;

		float partOfCircle = currentIndex * circleSlice;
		obj.m_Transform.SetPosition(sin(partOfCircle + totalTime), cos(partOfCircle + totalTime), 0);

		i++;
	}

}
