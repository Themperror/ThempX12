#include "ASEngine.h"

#include "core/engine.h"
#include "core/resources.h"
#include "core/util/Print.h"
#include "core/util/Break.h"

#include <lib/angelscript/angelscript.h>
#include <lib/angelscript/add_on/scriptstdstring/scriptstdstring.h>
#include <lib/angelscript/add_on/scriptbuilder/scriptbuilder.h>
#include <lib/angelscript/add_on/scriptarray/scriptarray.h>
#include <lib/angelscript/add_on/scriptdictionary/scriptdictionary.h>

namespace Themp::Scripting
{
	const uint64_t UserDataID = 0x1234C0FFEE1234; //completely arbitrary, just some big number unlikely to have been used by AngelScript internally

	void MessageCallback(const AngelScript::asSMessageInfo* msg, void* param)
	{
		const char* type = "ERR ";

		if (msg->type == AngelScript::asMSGTYPE_WARNING)
		{
			type = "WARN";
		}
		else if (msg->type == AngelScript::asMSGTYPE_INFORMATION)
		{
			type = "INFO";
		}

		Themp::Print("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
	}
	void ASPrint(const std::string& msg)
	{
		Themp::Print(msg);
	}
	void ASPrintIP0(const std::string& msg, int param0)
	{
		Themp::Print(msg, param0);
	}

	void ASPrintFP0(const std::string& msg, float param0)
	{
		Themp::Print(msg, param0);
	}

	void ASPrintSP0(const std::string& msg, std::string& param0)
	{
		Themp::Print(msg, param0.c_str());
	}

	static void ASYield()
	{
		// Get a pointer to the context that is currently being executed
		AngelScript::asIScriptContext* ctx = AngelScript::asGetActiveContext();
		if (ctx)
		{
			ctx->Suspend();
		}
	}

	static void ASCreateCoRoutine(AngelScript::asIScriptFunction* func)
	{
		if (func == 0)
			return;

		AngelScript::asIScriptContext* ctx = AngelScript::asGetActiveContext();
		if (ctx)
		{
			Script& script = *reinterpret_cast<Script*>(ctx->GetUserData(UserDataID));

			AngelScript::asIScriptEngine* engine = ctx->GetEngine();
			AngelScript::asIScriptContext* coroutine = engine->RequestContext();
			if (coroutine == nullptr)
			{
				Themp::Print("Failed to create coroutine from script %s", script.scriptName);
				Themp::Break();
			}

			// Prepare the context
			int r = coroutine->Prepare(func);
			if (r < 0)
			{
				// Couldn't prepare the context
				engine->ReturnContext(coroutine);
				Themp::Print("Failed to prepare coroutine from script %s", script.scriptName);
				Themp::Break();
				return;
			}
			//link our coroutine to our current script, so that if it creates other coroutines they all contain to the same script
			coroutine->SetUserData(&script, UserDataID);

			r = coroutine->Execute();
			if (r == AngelScript::asEContextState::asEXECUTION_SUSPENDED)
			{
				//we suspended so the coroutine isn't done yet, we'll add it to our list and continue it later
				script.coroutines.push_back(coroutine);
			}
		}
	}

	static void ASCreateCoRoutineWithDictionary(AngelScript::asIScriptFunction* func, AngelScript::CScriptDictionary* arg)
	{
		if (func == 0)
			return;

		AngelScript::asIScriptContext* ctx = AngelScript::asGetActiveContext();
		if (ctx)
		{
			Script& script = *reinterpret_cast<Script*>(ctx->GetUserData(UserDataID));

			AngelScript::asIScriptEngine* engine = ctx->GetEngine();
			AngelScript::asIScriptContext* coroutine = engine->RequestContext();
			if (coroutine == nullptr)
			{
				Themp::Print("Failed to create coroutine from script %s", script.scriptName);
				Themp::Break();
			}

			// Prepare the context
			int r = coroutine->Prepare(func);
			if (r < 0)
			{
				// Couldn't prepare the context
				engine->ReturnContext(coroutine);
				Themp::Print("Failed to prepare coroutine from script %s", script.scriptName);
				Themp::Break();
				return;
			}
			coroutine->SetArgObject(0, arg);
			//link our coroutine to our current script, so that if it creates other coroutines they all contain to the same script
			coroutine->SetUserData(&script, UserDataID); 
			
			r = coroutine->Execute();
			if (r == AngelScript::asEContextState::asEXECUTION_SUSPENDED)
			{
				//we suspended so the coroutine isn't done yet, we'll add it to our list and continue it later
				script.coroutines.push_back(coroutine);
			}
		}
	}

	bool ASEngine::Init()
	{
		m_ScriptingEngine = AngelScript::asCreateScriptEngine();
		int result = m_ScriptingEngine->SetMessageCallback(AngelScript::asFUNCTION(MessageCallback), nullptr, AngelScript::asCALL_CDECL);
		if (result < 0)
		{
			Themp::Print("Failed to set message callback on ASEngine");
			Themp::Break();
		}

		AngelScript::RegisterStdString(m_ScriptingEngine);

		result = m_ScriptingEngine->RegisterGlobalFunction("void print(const string &in)", AngelScript::asFUNCTION(ASPrint), AngelScript::asCALL_CDECL);
		if (result < 0)
		{
			Themp::Print("Failed to register print function to ASEngine");
			Themp::Break();
		}
		
		result = m_ScriptingEngine->RegisterGlobalFunction("void print(const string &in, int param0)", AngelScript::asFUNCTION(ASPrintIP0), AngelScript::asCALL_CDECL);
		if (result < 0)
		{
			Themp::Print("Failed to register print function to ASEngine");
			Themp::Break();
		}
		result = m_ScriptingEngine->RegisterGlobalFunction("void print(const string &in, float param0)", AngelScript::asFUNCTION(ASPrintFP0), AngelScript::asCALL_CDECL);
		if (result < 0)
		{
			Themp::Print("Failed to register print function to ASEngine");
			Themp::Break();
		}
		result = m_ScriptingEngine->RegisterGlobalFunction("void print(const string &in, string param0)", AngelScript::asFUNCTION(ASPrintSP0), AngelScript::asCALL_CDECL);
		if (result < 0)
		{
			Themp::Print("Failed to register print function to ASEngine");
			Themp::Break();
		}

		AngelScript::RegisterScriptArray(m_ScriptingEngine, true);
		AngelScript::RegisterScriptDictionary(m_ScriptingEngine);
		result = m_ScriptingEngine->RegisterGlobalFunction("void yield()", AngelScript::asFUNCTION(ASYield), AngelScript::asCALL_CDECL);
		result = m_ScriptingEngine->RegisterFuncdef("void coroutineDictionary(dictionary@)");
		result = m_ScriptingEngine->RegisterGlobalFunction("void createCoRoutine(coroutineDictionary @+, dictionary @+)", AngelScript::asFUNCTION(ASCreateCoRoutineWithDictionary), AngelScript::asCALL_CDECL);
		result = m_ScriptingEngine->RegisterFuncdef("void coroutine()");
		result = m_ScriptingEngine->RegisterGlobalFunction("void createCoRoutine(coroutine @+)", AngelScript::asFUNCTION(ASCreateCoRoutine), AngelScript::asCALL_CDECL);

		return m_ScriptingEngine != nullptr;
	}

	void ASEngine::CompileScripts()
	{
		const auto& scripts = Themp::Engine::instance->m_Resources->GetScriptFiles();
		AngelScript::CScriptBuilder builder;
		for (auto& pair : scripts)
		{
			int r = builder.StartNewModule(m_ScriptingEngine, pair.first.c_str());
			if (r < 0)
			{
				Themp::Print("Failed to start new scripting module");
				Themp::Break();
			}

			r = builder.AddSectionFromMemory(pair.first.c_str(), pair.second.c_str(), pair.second.size());
			if (r == 0)
			{
				Themp::Print("Script: %s was already included before", pair.first.c_str());
			}
			else if (r < 0)
			{
				Themp::Print("Failed to add script: %s", pair.first.c_str());
				Themp::Break();
			}

			r = builder.BuildModule();
			if (r < 0)
			{
				Themp::Print("Failed to build script: %s", pair.first.c_str());
				Themp::Break();
			}

			AngelScript::asIScriptModule* mod = m_ScriptingEngine->GetModule(pair.first.c_str());
			if (mod != nullptr)
			{
				AngelScript::asIScriptFunction* entryPoint = mod->GetFunctionByName("main");
				if (entryPoint == nullptr)
				{
					Themp::Print("No function named `main` was found in script: %s, please add the entry point and try again!", pair.first.c_str());
				}
				else
				{
					AngelScript::asIScriptContext* context = m_ScriptingEngine->CreateContext();
				
					Script script;
					script.context = context;
					script.scriptName = pair.first;

					r = script.context->Prepare(entryPoint);
					if (r < 0)
					{
						Themp::Print("Failed to prepare script: %s!", pair.first.c_str());
					}
					m_Scripts.push_back(script);
				}
			}
		}

		for (auto& script : m_Scripts)
		{
			script.context->SetUserData(&script, UserDataID);
		}
	}

	void ASEngine::Update()
	{
		for (auto& script : m_Scripts)
		{
			AngelScript::asEContextState state = script.context->GetState();
			if (state == AngelScript::asEContextState::asEXECUTION_PREPARED || state == AngelScript::asEContextState::asEXECUTION_SUSPENDED)
			{
				int ret = script.context->Execute();
				if (ret == AngelScript::asEContextState::asEXECUTION_EXCEPTION)
				{
					Themp::Print("Script %s has run into an exception!", script.scriptName.c_str());
					Themp::Break();
				}
				
			}

			if (state == AngelScript::asEContextState::asEXECUTION_SUSPENDED || state == AngelScript::asEContextState::asEXECUTION_FINISHED)
			{
				for (int i = 0; i < script.coroutines.size(); i++)
				{
					AngelScript::asIScriptContext* coroutine = script.coroutines[i];
					if (coroutine->GetState() == AngelScript::asEContextState::asEXECUTION_SUSPENDED)
					{
						int ret = coroutine->Execute();
						if (ret != AngelScript::asEXECUTION_FINISHED)
						{
							if (ret == AngelScript::asEContextState::asEXECUTION_EXCEPTION)
							{
								Themp::Print("Coroutine in script %s has run into an exception!", script.scriptName.c_str());
								Themp::Break();
							}
						}
						else
						{
							m_CoroutinesToRemove.push_back(coroutine);
						}
					}
				}

				for (auto& coroutineToRemove : m_CoroutinesToRemove)
				{
					for (int i = script.coroutines.size() - 1; i >= 0; i--)
					{
						if (coroutineToRemove == script.coroutines[i])
						{
							script.coroutines.erase(script.coroutines.begin() + i);
						}
					}
				}
				m_CoroutinesToRemove.clear();
			}
		}
	}

	void ASEngine::Stop()
	{
		int result = m_ScriptingEngine->ShutDownAndRelease();
		if (result < 0)
		{
			Themp::Print("Failed to register print function to ASEngine");
			Themp::Break();
		}
	}
}
