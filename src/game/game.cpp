#include "game/game.h"

#include <lib/imgui/imgui.h>
#include "core/renderer/gpu_resources.h"
#include "core/renderer/control.h"
#include "core/engine.h"

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

}
