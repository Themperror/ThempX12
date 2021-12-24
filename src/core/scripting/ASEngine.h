#pragma once

#include <vector>
#include <string>

namespace AngelScript
{
	class asIScriptEngine;
	class asIScriptContext;
}
namespace Themp::Scripting
{

	struct Script
	{
		std::string scriptName;
		AngelScript::asIScriptContext* context;
		std::vector<AngelScript::asIScriptContext*> coroutines;
	};

	class ASEngine
	{
	public:
		bool Init();
		void CompileScripts();
		void Update();
		void Stop();
	private:
		AngelScript::asIScriptEngine* m_ScriptingEngine = nullptr;
		std::vector<Script> m_Scripts;


		//reusable storage to prevent allocations every Update();
		std::vector<AngelScript::asIScriptContext*> m_CoroutinesToRemove;
	};
}