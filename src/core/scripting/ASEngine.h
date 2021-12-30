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
namespace Themp::D3D
{
	struct RenderPass;
}
namespace Themp::Scripting
{
	struct Script
	{
		ScriptHandle handle;
		std::string scriptName;
		AngelScript::asIScriptContext* context;
		std::vector<AngelScript::asIScriptContext*> coroutines;
	};

	class ASEngine
	{
	public:
		bool Init();
		ScriptHandle AddScript(const std::string& filename);
		void LinkToSceneObject(ScriptHandle handle, std::string& name);
		void LoadScript(Script& script, std::string data);
		Script* GetScript(ScriptHandle handle);
		ScriptHandle GetNextScriptHandle();

		void CompileScripts();
		void Update(Resources& resources, const std::vector<Themp::D3D::RenderPass>& renderPasses);
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