#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOMENUS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <Windows.h>

#include <map>
#include <memory>
#include <variant>
#include <string>

namespace Game
{
	class Game;
}
namespace Themp
{
	namespace D3D
	{
		class Control;
	}
	namespace Input
	{
		class Manager;
	}
	class Resources;
	class Engine
	{
	public:
		static std::unique_ptr<Themp::Engine> instance;
		void Start();

		enum class SVar 
		{ 
			SVAR_iWindowWidth, 
			SVAR_iWindowHeight, 
			SVAR_iFullScreen, 
			SVAR_iWindowPosX, 
			SVAR_iWindowPosY, 
			SVAR_iAnisotropic_Filtering,
			SVAR_iNumBackBuffers,
			SVAR_iVSyncEnabled,
			COUNT,
		};

		const char* const s_SVarStr[static_cast<size_t>(SVar::COUNT)] =
		{
			"iWindowSizeX",
			"iWindowSizeY",
			"iFullscreen",
			"iWindowPosX",
			"iWindowPosY",
			"iAnisotropic_Filtering",
			"iNumBackBuffers",
			"iVsyncEnabled"
		};
		
		const char* const GetStringSVar(SVar svar) const 
		{
			return s_SVarStr[static_cast<size_t>(svar)];
		}


		int GetSVarInt(SVar svar);
		float GetSVarFloat(SVar svar);
		void SetSVarInt(SVar svar, int val);
		void SetSVarFloat(SVar svar, float val);

		std::string m_BaseDir;
		HWND m_Window = nullptr;
		HINSTANCE m_HInstance = 0;
		bool m_Quitting = false;
		bool m_CursorShown = true;
		std::map<SVar, std::variant<int,float>> m_SVars;
		std::unique_ptr<Game::Game> m_Game;
		std::unique_ptr<D3D::Control> m_Renderer;
		std::unique_ptr<Resources> m_Resources;
		std::unique_ptr<Input::Manager> m_Input;

	private:
		
	};
};