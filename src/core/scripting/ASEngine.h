#pragma once

#include <vector>
#include <string>
#include "core/renderer/types.h"

namespace AngelScript
{
	class asIScriptEngine;
	class asIScriptContext;
}

namespace Themp
{
	class Resources;
}
namespace Themp::Scripting
{

	struct Script
	{
		D3D::ScriptHandle handle;
		std::string scriptName;
		AngelScript::asIScriptContext* context;
		std::vector<AngelScript::asIScriptContext*> coroutines;
	};

	class ASEngine
	{
	public:
		bool Init();
		D3D::ScriptHandle AddScript(const std::string& filename);
		void LinkToObject3D(D3D::ScriptHandle handle, std::string& name);
		void LoadScript(Script& script, std::string data);
		Script* GetScript(D3D::ScriptHandle handle);
		D3D::ScriptHandle GetNextScriptHandle();

		void CompileScripts();
		void Update(Resources& resources);
		void Stop();
	private:
		void ExecuteScript(Script& script);
		AngelScript::asIScriptEngine* m_ScriptingEngine = nullptr;
		std::vector<Script> m_Scripts;
		size_t m_LatestHandleID = 0;

		//reusable storage to prevent allocations every Update();
		std::vector<AngelScript::asIScriptContext*> m_CoroutinesToRemove;
	};
}