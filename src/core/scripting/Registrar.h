#pragma once
#include "core/renderer/types.h"
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
		static void SetCurrentRenderPass(D3D::RenderPassHandle handle);
	private:
		static void RegisterGlobalFunctions(AngelScript::asIScriptEngine* scriptingEngine);
		static void RegisterMemberFunctions(AngelScript::asIScriptEngine* scriptingEngine);
	};

}