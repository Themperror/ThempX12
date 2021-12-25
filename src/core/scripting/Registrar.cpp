#include "Registrar.h"
#include "core/util/print.h"
#include "core/util/break.h"

#include <string>
#include <lib/angelscript/angelscript.h>

#define CheckResult(result, funcName) \
	if ((result) < 0) { \
	Themp::Print("Failed to register" #funcName " function to ASEngine"); \
	Themp::Break();}

namespace Themp::Scripting
{
	void ASPrint(const std::string& msg)
	{
		Themp::Print(msg);
	}

	void ASPrint(const std::string& msg, int param0)
	{
		Themp::Print(msg, param0);
	}

	void ASPrint(const std::string& msg, float param0)
	{
		Themp::Print(msg, param0);
	}

	void ASPrint(const std::string& msg, std::string& param0)
	{
		Themp::Print(msg, param0.c_str());
	}


	void Registrar::Init(AngelScript::asIScriptEngine* scriptingEngine)
	{
		RegisterGlobalFunctions(scriptingEngine);
		RegisterMemberFunctions(scriptingEngine);
	}
	void Registrar::RegisterGlobalFunctions(AngelScript::asIScriptEngine* scriptingEngine)
	{
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&), void), AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in, int param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, int), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in, float param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, float), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in, string param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, std::string&), void) , AngelScript::asCALL_CDECL), ASPrint);


	}
	void Registrar::RegisterMemberFunctions(AngelScript::asIScriptEngine* scriptingEngine)
	{

	}
}