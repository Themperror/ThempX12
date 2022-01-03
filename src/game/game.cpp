#include "game/game.h"

#include <lib/imgui/imgui.h>
#include "core/renderer/gpu_resources.h"
#include "core/renderer/control.h"
#include "core/engine.h"
#include "core/input/manager.h"
#include "core/input/keyboard.h"
#include "core/resources.h"
#include "core/scripting/asengine.h"
#include "core/util/svars.h"

void Game::Game::Start()
{
	Themp::D3D::Control& renderer = *Themp::Engine::instance->m_Renderer;
	//renderer.AddSubPass(mainHandle, 0);
	//renderer.Register(mainHandle, subHandle, model);
	//
	//auto subPass = m_MainPass.AddSubPass(0); // AddSubPass(model.GetShadowMaterial())
	//subPass.Register(model);
	m_Camera = std::make_unique<Themp::Camera>();

	float scrWidth =  static_cast<float>(Themp::Engine::instance->s_SVars.GetSVarInt(Themp::SVar::iWindowWidth));
	float scrHeight = static_cast<float>(Themp::Engine::instance->s_SVars.GetSVarInt(Themp::SVar::iWindowHeight));

	m_Camera->SetPosition(0, 0, -10);
	m_Camera->SetTarget(DirectX::XMFLOAT3(0, 0, 0));
	m_Camera->SetAspectRatio(scrWidth / scrHeight);
	m_Camera->SetFoV(75);
	m_Camera->SetProjection(Themp::Camera::CameraType::Perspective);
	m_Camera->SetNear(0.1f);
	m_Camera->SetFar(1000.0f);
	m_Camera->Rotate(0, 0);
}

void Game::Game::Stop()
{

}

float totalMouseX = 0;
float totalMouseY = 0;

void Game::Game::Update(double delta)
{
	using namespace Themp::Input;
	const Themp::Input::Keyboard& keyboard = Themp::Engine::instance->m_Input->GetDevice<Themp::Input::Keyboard>(0);
	const auto& keyboardState = keyboard.GetState();
	//const Themp::Input::Mouse& mouse = Themp::Engine::instance->m_Input->GetDevice<Themp::Input::Mouse>(0);
	//left mouse button
	if (keyboardState.keys[VK_LEFT] == Keyboard::ButtonState::Down ||
		keyboardState.keys[VK_RIGHT] == Keyboard::ButtonState::Down ||
		keyboardState.keys[VK_UP] == Keyboard::ButtonState::Down ||
		keyboardState.keys[VK_DOWN] == Keyboard::ButtonState::Down)
	{
		if (keyboardState.keys[VK_LEFT] == Keyboard::ButtonState::Down)
		{
			totalMouseX += 30.0 * delta;
		}
		if (keyboardState.keys[VK_RIGHT] == Keyboard::ButtonState::Down)
		{
			totalMouseX -= 30.0 * delta;
		}
		if (keyboardState.keys[VK_UP] == Keyboard::ButtonState::Down)
		{
			totalMouseY += 30.0 * delta;
		}
		if (keyboardState.keys[VK_DOWN] == Keyboard::ButtonState::Down)
		{
			totalMouseY -= 30.0 * delta;
		}

		totalMouseY = totalMouseY > 90.0f ? 90.0f : totalMouseY < -90.0f ? -90.0f : totalMouseY;
		m_Camera->Rotate(totalMouseX, totalMouseY);
	}
	float speedMod = 0.05f;
	if (keyboardState.keys[VK_SHIFT] == Keyboard::ButtonState::Down)
	{
		speedMod = 0.2f;
	}
	m_Camera->SetSpeed(speedMod);
	if (keyboardState.keys['W'] == Keyboard::ButtonState::Down)
	{
		m_Camera->MoveForward();
	}
	if (keyboardState.keys['S'] == Keyboard::ButtonState::Down)
	{
		m_Camera->MoveBackward();
	}
	if (keyboardState.keys['A'] == Keyboard::ButtonState::Down)
	{
		m_Camera->MoveLeft();
	}
	if (keyboardState.keys['D'] == Keyboard::ButtonState::Down)
	{
		m_Camera->MoveRight();
	}
	//spacebar
	if (keyboardState.keys[VK_SPACE] == Keyboard::ButtonState::Down || keyboardState.keys['E'] == Keyboard::ButtonState::Down)
	{
		m_Camera->MoveUp();
	}
	if (keyboardState.keys['X'] == Keyboard::ButtonState::Down || keyboardState.keys['Q'] == Keyboard::ButtonState::Down)
	{
		m_Camera->MoveDown();
	}

	m_Camera->Update((float)delta);
	m_Camera->UpdateMatrices();

	Themp::D3D::ConstantBufferHandle camBufHandle = Themp::Engine::instance->m_Renderer->GetCameraConstantBuffer();
	Themp::Engine::instance->m_Renderer->GetResourceManager().UpdateCameraConstantBuffer(camBufHandle, m_Camera->m_CameraConstantBufferData);


	auto& sceneObjs = Themp::Engine::instance->m_Resources->GetSceneObjects();
	int i = 0;
	float tau = 3.1415f * 2.0f;
	float circleSlice = (tau / (float)sceneObjs.size());
	float totalTime = Themp::Engine::instance->GetTimeSinceLaunch();
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

	//for (auto& obj : sceneObjs)
	//{
	//	float currentIndex = i;
	//	if (currentIndex == 5) break;
	//	float partOfCircle = currentIndex * circleSlice;
	//	obj.m_Transform.SetPosition(sin(partOfCircle + totalTime), cos(partOfCircle + totalTime), 0);
	//
	//	i++;
	//}
}
