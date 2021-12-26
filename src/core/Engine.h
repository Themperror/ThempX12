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
#define VC_EXTRALEAN
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
	namespace Scripting
	{
		class ASEngine;
	}
	class Resources;
	class SVars;
	class Engine
	{
	public:
		static std::unique_ptr<Themp::Engine> instance;
		static SVars s_SVars;
		void Start();
		void ResizeWindow(int width, int height);
		static std::string ReadFileToString(const std::string& filePath);

		std::string m_BaseDir;
		HWND m_Window = nullptr;
		HINSTANCE m_HInstance = 0;
		bool m_Quitting = false;
		bool m_CursorShown = true;
		bool m_AllowResizing = true;
		std::unique_ptr<Game::Game> m_Game;
		std::unique_ptr<Scripting::ASEngine> m_Scripting;
		std::unique_ptr<D3D::Control> m_Renderer;
		std::unique_ptr<Resources> m_Resources;
		std::unique_ptr<Input::Manager> m_Input;

	private:
		
	};
};