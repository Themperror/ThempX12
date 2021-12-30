#include "engine.h"

#include "core/util/timer.h"
#include "core/Resources.h"
#include "core/renderer/control.h"
#include "core/renderer/types.h"
#include "core/input/keyboard.h"
#include "core/input/manager.h"
#include "core/scripting/ASEngine.h"
#include "core/util/print.h"
#include "core/util/break.h"
#include "core/util/svars.h"
#include "game/game.h"


#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>


#include <lib/imgui/imgui.h>
#include <lib/imgui/impl/imgui_impl_dx12.h>
#include <lib/imgui/impl/imgui_impl_win32.h>


using namespace Themp;
#pragma warning( disable : 4996) //disables warning unsafe function: freopen() fopen() .. etc
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
DEVMODE dm = { 0 };

namespace Themp
{
	std::unique_ptr<Themp::Engine> Engine::instance;
	SVars Engine::s_SVars;

	void Engine::Start()
	{
		srand((uint32_t)time(nullptr));
		Print("Creating Managers!");
		m_Renderer = std::make_unique<D3D::Control>();
		m_Resources = std::make_unique<Resources>();
		m_Game = std::make_unique<Game::Game>();
		m_Input = std::make_unique<Input::Manager>();
		m_Scripting = std::make_unique<Scripting::ASEngine>();

		ImGui::CreateContext();
		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		io.DisplaySize.x = static_cast<float>(Engine::s_SVars.GetSVarInt(SVar::iWindowWidth));
		io.DisplaySize.y = static_cast<float>(Engine::s_SVars.GetSVarInt(SVar::iWindowHeight));
		ImGui_ImplWin32_Init(m_Window);

		Print("Initialising D3D12!");
		instance->m_Quitting = !m_Renderer->Init();
		if (instance->m_Quitting)
		{
			Print("Failed to set up D3D12!"); 
			MessageBox(m_Window, "Failed to initialise all required D3D12 resources, Is your hardware supported?", "Engine - Critical Error", MB_OK); 
		}
		m_Input->AddInputDevice<Input::Keyboard>(0);

		m_Scripting->Init();

		Print("Reading all resource data!");
		m_Resources->LoadMaterials();
		m_Renderer->CreatePipelines(*m_Resources);
		m_Resources->LoadScene("testScene.scene");

		m_Renderer->PopulateRenderingGraph(*m_Resources);

		Print("Setting up Game!");
		m_Game->Start();


		Timer mainTimer;
		Timer tickTimer;
		Timer drawTimer;
		mainTimer.StartTime();
		tickTimer.StartTime();
		drawTimer.StartTime();
		double trackerTime = 0;
		int numSamples = 0;
		double frameTimeAdd = 0, tickTimeAdd=0;

		RECT windowRect,clientRect;
		GetWindowRect(m_Window, &windowRect);
		GetClientRect(m_Window, &clientRect);


		//m_Renderer->ResizeWindow(clientRect.right, clientRect.bottom);
		//printf("BorderX: %i\n BorderY: %i\n Caption: %i\n", borderX, borderY, caption);
		SetCursorPos(windowRect.left + (windowRect.right - windowRect.left) / 2, windowRect.top + (windowRect.bottom - windowRect.top) / 2);
		const int captionSize = GetSystemMetrics(SM_CYCAPTION);
		const int frameSizeY =  GetSystemMetrics(SM_CYFIXEDFRAME);
		const int frameSizeX =  GetSystemMetrics(SM_CXFIXEDFRAME);
		const int borderSizeY = GetSystemMetrics(SM_CYEDGE);
		const int borderSizeX = GetSystemMetrics(SM_CXEDGE);
		Print("Starting main loop!");
		ShowCursor(true);
		m_CursorShown = true;
		while (!instance->m_Quitting)
		{
			m_AllowResizing = true;
			Input::Keyboard& mainKeyboard = m_Input->GetDevice<Input::Keyboard>(0);
			double delta = mainTimer.GetDeltaTimeReset();

			m_TimeSinceLaunch += delta;
			m_DeltaTime = delta;

			trackerTime += delta;

			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
				{
					instance->m_Quitting = true;
				}
				if (msg.message == WM_KEYUP)
				{
					if (msg.wParam > 256)continue;
					mainKeyboard.m_State.keys[msg.wParam] = Input::Keyboard::ButtonState::JustUp;
				}
				if (msg.message == WM_LBUTTONUP)
				{
					//m_Game->m_Keys[256] = -1;
				}
				if (msg.message == WM_RBUTTONUP)
				{
					//m_Game->m_Keys[257] = -1;
				}

				if (msg.message == WM_LBUTTONDOWN)
				{
					//if (m_Game->m_Keys[256] <= 0)
					//{
					//	m_Game->m_Keys[256] = 2;
					//}
				}
				if (msg.message == WM_RBUTTONDOWN)
				{
					//if (m_Game->m_Keys[257] <= 0)
					//{
					//	m_Game->m_Keys[257] = 2;
					//}
				}
				
				if (msg.message == WM_KEYDOWN)
				{
					if (msg.wParam >= 256)continue;

					if (mainKeyboard.m_State.keys[msg.wParam] != Input::Keyboard::ButtonState::Down) //repeat calls
					{
						mainKeyboard.m_State.keys[msg.wParam] = Input::Keyboard::ButtonState::JustDown;
					}
				}
				
			}

			{
				{
					GetWindowRect(m_Window, &windowRect);
					POINT cursorPos;
					//sadly we need all these calls
					GetClientRect(m_Window, &clientRect);
					GetCursorPos(&cursorPos);
					ScreenToClient(m_Window, &cursorPos);
					int windowDiffX = (windowRect.right - windowRect.left - clientRect.right) / 2;
					int windowDiffY = (int)((windowRect.bottom - windowRect.top - clientRect.bottom) * 0.75);
					//int WindowedMouseX = cursorPos.x - windowRect.left - windowDiffX;
					//int WindowedMouseY = cursorPos.y - windowRect.top - windowDiffY;
					io.DeltaTime = (float)delta;
					//windows Title bar messes up the actual mouse position for collision testing with the UI, so I adjust it to fit "good enough" since getting exact measurements from top and bottom is a pain
					io.MousePos = ImVec2((float)cursorPos.x, (float)cursorPos.y);
					io.DisplaySize = ImVec2((float)clientRect.right, (float)clientRect.bottom);
				}
				ImGui_ImplDX12_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				tickTimer.StartTime();
				m_Game->Update(delta);
				m_Scripting->Update(*m_Resources, m_Renderer->GetRenderPasses());
				tickTimeAdd += tickTimer.GetDeltaTimeReset();

				m_Input->Update();

				drawTimer.StartTime();
				//Doesn't actually render but prepares render data for us to use

				//m_Renderer->m_ConstantBufferData.time = (float)time;
				//m_Renderer->dirtyEngineBuffer = true;
				//m_Renderer->PrepareEngineBuffer();
				m_Renderer->GetResourceManager().UploadMeshStagingBuffers();
				m_Renderer->BeginDraw();
				ImGui::Render();

				if(!instance->m_Quitting)
				{
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
				}

				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_Renderer->GetImguiCmdList().Get());

				m_Renderer->EndDraw();
				frameTimeAdd += drawTimer.GetDeltaTimeReset();

				numSamples++;
				if (trackerTime >= 1.0)
				{
					//display FPS and other info
					Themp::Print("Avg FPS: %5i  Avg Frametime: %.5f   Avg Tick Time: %.5f", numSamples, frameTimeAdd / (float)numSamples, tickTimeAdd/(float)numSamples);
					trackerTime = trackerTime - 1.0;
					frameTimeAdd = 0;
					tickTimeAdd = 0;
					numSamples = 0;
				}
			}
		}

		GetWindowRect(m_Window, &windowRect);
		Engine::s_SVars.SetSVarInt(SVar::iWindowWidth, windowRect.right - windowRect.left);
		Engine::s_SVars.SetSVarInt(SVar::iWindowHeight, windowRect.bottom - windowRect.top);

		m_Scripting->Stop();
		m_Game->Stop();
		m_Game = nullptr;
		m_Renderer->Stop();
		m_Resources = nullptr;
		m_Renderer = nullptr;
		if (!m_CursorShown)
		{
			m_CursorShown = true;
			ShowCursor(true);
		}
		ImGui::DestroyContext();
	}

	void Engine::ResizeWindow(int width, int height)
	{
		Themp::Print("Resizing Window to %ix%i !", width, height);
		if (m_Renderer)
		{
			m_Renderer->ResizeSwapchain(width, height);
		}
		if (m_Resources)
		{
			m_Resources->ResizeRendertargets(width, height);
		}
	}
}

std::string GetPathName(std::string s)
{
	std::string name = "";
	int64_t size = s.size() - 1;
	for (int64_t i = size; i >= 0; i--)
	{
		if (s.at(i) == '\\' || s[i] == '/')
		{
			i++;
			name = s.substr(0, i);
			break;
		}
	}
	return name;
}


std::string Engine::ReadFileToString(const std::string& filePath)
{
	HANDLE file = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE)
	{
		Themp::Print("Was unable to open the file: [%s]", filePath.c_str());
		Themp::Break();
		return "";
	}

	DWORD fileSize = GetFileSize(file, NULL);
	std::string data(fileSize, '\0');
	DWORD readBytes = 0;
	if (!ReadFile(file, data.data(), fileSize, &readBytes, NULL))
	{
		Themp::Print("Was unable to read the file: [%s]", filePath.c_str());
		Themp::Break();
	}
	if (readBytes != fileSize)
	{
		Themp::Print("Was unable to read the entire file: [%s]", filePath.c_str());
		Themp::Break();
	}
	CloseHandle(file);

	return data;
}


int newWindowSizeX = 0;
int newWindowSizeY = 0;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	AllocConsole();
	FILE* conout = freopen("CONOUT$", "w", stdout);

	Themp::Engine::instance = std::make_unique<Themp::Engine>();
	Themp::Engine* system = Themp::Engine::instance.get();

	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

	char szFileName[MAX_PATH + 1];
	GetModuleFileNameA(NULL, szFileName, MAX_PATH + 1);
	system->m_BaseDir = GetPathName(std::string(szFileName));

	Themp::Util::SetLogFile("log.txt");
		
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "ThempX12";

	RegisterClassEx(&wc);


	newWindowSizeX = Engine::s_SVars.GetSVarInt(SVar::iWindowWidth);
	newWindowSizeY = Engine::s_SVars.GetSVarInt(SVar::iWindowHeight);

	if (Engine::s_SVars.GetSVarInt(SVar::iFullScreen) == 1)
	{
		HWND desktop = GetDesktopWindow();
		RECT bSize;
		GetWindowRect(desktop, &bSize);
		
		
		system->m_Window = CreateWindowEx(0,
			wc.lpszClassName,
			"ThempX12",
			WS_OVERLAPPEDWINDOW,
			bSize.left,
			bSize.top,
			bSize.right,
			bSize.bottom,
			NULL,NULL,hInstance,NULL);
	}
	else
	{
		system->m_Window = CreateWindowEx(0,
			wc.lpszClassName,
			"ThempX12",
			WS_OVERLAPPEDWINDOW,
			Engine::s_SVars.GetSVarInt(SVar::iWindowPosX),
			Engine::s_SVars.GetSVarInt(SVar::iWindowPosY),
			newWindowSizeX,
			newWindowSizeY,
			NULL, NULL, hInstance, NULL);
	}


	ShowWindow(system->m_Window, nShowCmd);


	system->Start();

	Engine::s_SVars.Store();
	if(conout != nullptr)
		fclose(conout);

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	RECT windowRect;
	if (message == WM_SIZE)
	{
		if (wParam == SIZE_MAXIMIZED)
		{
			Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowPosX, 0);
			Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowPosY, 0);
		}
		if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		{
			GetWindowRect(Themp::Engine::instance->m_Window, &windowRect);

			newWindowSizeX = windowRect.right;
			newWindowSizeY = windowRect.bottom;

			GetClientRect(Themp::Engine::instance->m_Window, &windowRect);
			int currentWidth = Themp::Engine::s_SVars.GetSVarInt(SVar::iWindowWidth);
			int currentHeight = Themp::Engine::s_SVars.GetSVarInt(SVar::iWindowHeight);
			if (currentWidth != windowRect.right || currentHeight != windowRect.bottom)
			{
				if (Themp::Engine::instance->m_AllowResizing)
				{
					Themp::Engine::instance->m_AllowResizing = false;
					Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowWidth, windowRect.right);
					Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowHeight, windowRect.bottom);
					Themp::Engine::instance->ResizeWindow(windowRect.right, windowRect.bottom);
				}
			}			
		}
	}

	switch (message)
	{
		case WM_CLOSE:
		{
			Themp::Engine::instance->m_Quitting = true;
		}
		break;
		case WM_DESTROY:
		{
			Themp::Engine::instance->m_Quitting = true;
		}
		break;
		case WM_MOVING:
		{
			GetWindowRect(Themp::Engine::instance->m_Window, &windowRect);

			Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowPosX, windowRect.left);
			Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowPosY, windowRect.top);
		}
		break;
		case WM_ENTERSIZEMOVE:
		break;
		case WM_EXITSIZEMOVE:
			GetClientRect(Themp::Engine::instance->m_Window, &windowRect);
			Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowWidth, windowRect.right);
			Themp::Engine::s_SVars.SetSVarInt(SVar::iWindowHeight, windowRect.bottom);
			Themp::Engine::instance->ResizeWindow(windowRect.right, windowRect.bottom);
			
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}