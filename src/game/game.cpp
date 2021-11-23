#include "game.h"

#include <imgui.h>
#include "renderer/gpu_resources.h"
#include "renderer/control.h"
#include "engine.h"

void Game::Game::Start()
{
	auto model = Themp::Engine::instance->m_Renderer->GetResourceManager().Test_GetAndAddRandomModel();

	Themp::D3D::Control& renderer = *Themp::Engine::instance->m_Renderer;
	Themp::D3D::MainPassHandle mainHandle = Themp::Engine::instance->m_Renderer->AddMainPass("Main");
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

}
