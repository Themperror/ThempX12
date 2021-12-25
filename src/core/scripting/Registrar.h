#pragma once

namespace AngelScript
{
	class asIScriptEngine;
}
namespace Themp::Scripting
{
	class Registrar
	{
	public:
		static void Init(AngelScript::asIScriptEngine* scriptingEngine);
	private:
		static void RegisterGlobalFunctions(AngelScript::asIScriptEngine* scriptingEngine);
		static void RegisterMemberFunctions(AngelScript::asIScriptEngine* scriptingEngine);
	};

}