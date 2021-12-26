#include "ASEngine.h"

#include "core/engine.h"
#include "core/resources.h"
#include "core/util/Print.h"
#include "core/util/Break.h"
#include "core/scripting/registrar.h"

#include <lib/angelscript/angelscript.h>
#include <lib/angelscript/add_on/scriptstdstring/scriptstdstring.h>
#include <lib/angelscript/add_on/scriptbuilder/scriptbuilder.h>
#include <lib/angelscript/add_on/scriptarray/scriptarray.h>
#include <lib/angelscript/add_on/scriptdictionary/scriptdictionary.h>

namespace Themp::Scripting
{
	const uint64_t UserDataID = 0x1234C0FFEE1234; //completely arbitrary, just some big number unlikely to have been used by AngelScript internally


	std::string_view GetRetCodeAsString(AngelScript::asERetCodes retCode)
	{
		//asSUCCESS = 0,
		//asERROR = -1,
		//asCONTEXT_ACTIVE = -2,
		//asCONTEXT_NOT_FINISHED = -3,
		//asCONTEXT_NOT_PREPARED = -4,
		//asINVALID_ARG = -5,
		//asNO_FUNCTION = -6,
		//asNOT_SUPPORTED = -7,
		//asINVALID_NAME = -8,
		//asNAME_TAKEN = -9,
		//asINVALID_DECLARATION = -10,
		//asINVALID_OBJECT = -11,
		//asINVALID_TYPE = -12,
		//asALREADY_REGISTERED = -13,
		//asMULTIPLE_FUNCTIONS = -14,
		//asNO_MODULE = -15,
		//asNO_GLOBAL_VAR = -16,
		//asINVALID_CONFIGURATION = -17,
		//asINVALID_INTERFACE = -18,
		//asCANT_BIND_ALL_FUNCTIONS = -19,
		//asLOWER_ARRAY_DIMENSION_NOT_REGISTERED = -20,
		//asWRONG_CONFIG_GROUP = -21,
		//asCONFIG_GROUP_IS_IN_USE = -22,
		//asILLEGAL_BEHAVIOUR_FOR_TYPE = -23,
		//asWRONG_CALLING_CONV = -24,
		//asBUILD_IN_PROGRESS = -25,
		//asINIT_GLOBAL_VARS_FAILED = -26,
		//asOUT_OF_MEMORY = -27,
		//asMODULE_IS_IN_USE = -28

		std::string_view strings[29] =
		{
			"SUCCESS",
			"ERROR",
			"CONTEXT_ACTIVE",
			"CONTEXT_NOT_FINISHED",
			"CONTEXT_NOT_PREPARED",
			"INVALID_ARG",
			"NO_FUNCTION",
			"NOT_SUPPORTED",
			"INVALID_NAME",
			"NAME_TAKEN",
			"INVALID_DECLARATION",
			"INVALID_OBJECT",
			"INVALID_TYPE",
			"ALREADY_REGISTERED",
			"MULTIPLE_FUNCTIONS",
			"NO_MODULE",
			"NO_GLOBAL_VAR",
			"INVALID_CONFIGURATION",
			"INVALID_INTERFACE",
			"CANT_BIND_ALL_FUNCTIONS",
			"LOWER_ARRAY_DIMENSION_NOT_REGISTERED",
			"WRONG_CONFIG_GROUP",
			"CONFIG_GROUP_IS_IN_USE",
			"ILLEGAL_BEHAVIOUR_FOR_TYPE",
			"WRONG_CALLING_CONV",
			"BUILD_IN_PROGRESS",
			"INIT_GLOBAL_VARS_FAILED",
			"OUT_OF_MEMORY",
			"MODULE_IS_IN_USE",
		};
		return strings[std::abs(retCode)];
	}


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

	static void ASYield()
	{
		// Get a pointer to the context that is currently being executed
		AngelScript::asIScriptContext* ctx = AngelScript::asGetActiveContext();
		if (ctx)
		{
			ctx->Suspend();
		}
	}

	AngelScript::asEContextState ExecuteCoroutine(AngelScript::asIScriptContext& coroutine)
	{
		AngelScript::asERetCodes result = static_cast<AngelScript::asERetCodes>(coroutine.Execute());
		if (result < 0)
		{
			std::string_view errString = GetRetCodeAsString(result);
			Themp::Print("Coroutine executed with error! [%*s]", errString.size(), errString.data());
		}
		return coroutine.GetState();
	}

	AngelScript::asIScriptContext* CreateCoroutine(AngelScript::asIScriptFunction* func, Script*& script)
	{
		AngelScript::asIScriptContext* ctx = AngelScript::asGetActiveContext();
		if (ctx)
		{
			D3D::ScriptHandle handle = reinterpret_cast<size_t>(ctx->GetUserData(UserDataID));
			script = Engine::instance->m_Scripting->GetScript(handle);


			AngelScript::asIScriptEngine* engine = ctx->GetEngine();
			AngelScript::asIScriptContext* coroutine = engine->RequestContext();
			if (coroutine == nullptr)
			{
				Themp::Print("Failed to create coroutine from script %s", script->scriptName);
				Themp::Break();
				return nullptr;
			}

			// Prepare the context
			if (coroutine->Prepare(func) < 0)
			{
				// Couldn't prepare the context
				engine->ReturnContext(coroutine);
				Themp::Print("Failed to prepare coroutine from script %s", script->scriptName);
				Themp::Break();
				return nullptr;
			}
			//link our coroutine to our current script, so that if it creates other coroutines they all contain to the same script
			coroutine->SetUserData(reinterpret_cast<void*>(handle.handle), UserDataID);

			return coroutine;
		}

		return nullptr;
	}

	void ExecuteAndAddCoroutine(AngelScript::asIScriptContext& coroutine, Script*& script)
	{
		AngelScript::asEContextState result = ExecuteCoroutine(coroutine);
		if (result == AngelScript::asEContextState::asEXECUTION_SUSPENDED)
		{
			//we suspended so the coroutine isn't done yet, we'll add it to our list and continue it later
			script->coroutines.push_back(&coroutine);
		}
		else
		{
			if (result == AngelScript::asEContextState::asEXECUTION_EXCEPTION)
			{
				Themp::Print("coroutine in script %s has run into an exception! [Function: %s] [Line: %i] [Exception: %s]", script->scriptName.c_str(), coroutine.GetExceptionFunction()->GetName(), coroutine.GetExceptionLineNumber(), coroutine.GetExceptionString());
				Themp::Break();
			}
			coroutine.GetEngine()->ReturnContext(&coroutine);
		}
	}

	static void ASCreateCoroutine(AngelScript::asIScriptFunction* func)
	{
		if (func == 0)
			return;

		Script* script = nullptr;
		AngelScript::asIScriptContext* coroutine = CreateCoroutine(func, script);
		
		ExecuteAndAddCoroutine(*coroutine, script);
	}

	static void ASCreateCoroutineWithDictionary(AngelScript::asIScriptFunction* func, AngelScript::CScriptDictionary* arg)
	{
		if (func == 0)
			return;

		Script* script = nullptr;
		AngelScript::asIScriptContext* coroutine = CreateCoroutine(func, script);

		if (coroutine->SetArgObject(0, arg) < 0)
		{
			Themp::Print("Failed to set first argument from script %s, does it have a first argument?", script->scriptName);
			Themp::Break();
		}

		ExecuteAndAddCoroutine(*coroutine, script);
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
		AngelScript::RegisterScriptArray(m_ScriptingEngine, true);
		AngelScript::RegisterScriptDictionary(m_ScriptingEngine);

		result = m_ScriptingEngine->RegisterGlobalFunction("void yield()", AngelScript::asFUNCTION(ASYield), AngelScript::asCALL_CDECL);
		result = m_ScriptingEngine->RegisterFuncdef("void coroutineDictionary(dictionary@)");
		result = m_ScriptingEngine->RegisterGlobalFunction("void createCoroutine(coroutineDictionary @+, dictionary @+)", AngelScript::asFUNCTION(ASCreateCoroutineWithDictionary), AngelScript::asCALL_CDECL);
		result = m_ScriptingEngine->RegisterFuncdef("void coroutine()");
		result = m_ScriptingEngine->RegisterGlobalFunction("void createCoroutine(coroutine @+)", AngelScript::asFUNCTION(ASCreateCoroutine), AngelScript::asCALL_CDECL);

		Registrar::Init(m_ScriptingEngine);
		return m_ScriptingEngine != nullptr;
	}

	D3D::ScriptHandle ASEngine::AddScript(const std::string& filename)
	{
		Script script{};
		script.scriptName = filename;
		LoadScript(script, Engine::ReadFileToString(std::string(Resources::GetScriptsFolder()).append(filename).append(".as")));
		script.handle = GetNextScriptHandle();
		m_Scripts.push_back(script);

		script.context->SetUserData(reinterpret_cast<void*>(script.handle.handle), UserDataID);
		return script.handle;
	}

	void ASEngine::LinkToObject3D(D3D::ScriptHandle handle, std::string& name)
	{
		for (int i = 0; i < m_Scripts.size(); i++)
		{
			if(m_Scripts[i].handle == handle.handle)
			{
				if (m_Scripts[i].context->SetArgObject(0, &name) < 0)
				{
					Themp::Print("Failed to set argument to script %s, does it not take an first argument in main?", m_Scripts[i].scriptName.c_str());
				}
			}
		}
	}

	Script* ASEngine::GetScript(D3D::ScriptHandle handle)
	{
		for (int i = 0; i < m_Scripts.size(); i++)
		{
			if (m_Scripts[i].handle == handle.handle)
			{
				return &m_Scripts[i];
			}
		}
		return nullptr;
	}

	D3D::ScriptHandle ASEngine::GetNextScriptHandle()
	{
		return m_LatestHandleID++;
	}

	void ASEngine::LoadScript(Script& script, std::string data)
	{
		AngelScript::CScriptBuilder builder;
		std::string moduleID = std::to_string(m_LatestHandleID);
		int r = builder.StartNewModule(m_ScriptingEngine, moduleID.c_str());
		if (r < 0)
		{
			Themp::Print("Failed to start new scripting module");
			Themp::Break();
		}

		r = builder.AddSectionFromMemory(script.scriptName.c_str(), data.c_str(), static_cast<unsigned int>(data.size()));
		if (r == 0)
		{
			Themp::Print("Script: %s was already included before", script.scriptName.c_str());
		}
		else if (r < 0)
		{
			Themp::Print("Failed to add script: %s", script.scriptName.c_str());
			Themp::Break();
		}

		r = builder.BuildModule();
		if (r < 0)
		{
			Themp::Print("Failed to build script: %s", script.scriptName.c_str());
			Themp::Break();
		}

		AngelScript::asIScriptModule* mod = m_ScriptingEngine->GetModule(moduleID.c_str());
		if (mod != nullptr)
		{
			AngelScript::asIScriptFunction* entryPoint = mod->GetFunctionByName("main");
			if (entryPoint == nullptr)
			{
				Themp::Print("No function named `main` was found in script: %s, please add the entry point and try again!", script.scriptName.c_str());
			}
			else
			{
				AngelScript::asIScriptContext* context = m_ScriptingEngine->CreateContext();

				script.context = context;
				r = script.context->Prepare(entryPoint);
				if (r < 0)
				{
					Themp::Print("Failed to prepare script: %s!", script.scriptName.c_str());
				}

			}
		}
	}

	void ASEngine::Update(Resources& resources)
	{
		for (auto& obj3D : resources.GetSceneObjects())
		{
			if (obj3D.m_ScriptHandle != D3D::ScriptHandle::Invalid)
			{
				Script* script = GetScript(obj3D.m_ScriptHandle);
				ExecuteScript(*script);
			}
		}
	}

	void ASEngine::ExecuteScript(Script& script)
	{
		AngelScript::asEContextState state = script.context->GetState();
		if (state == AngelScript::asEContextState::asEXECUTION_PREPARED || state == AngelScript::asEContextState::asEXECUTION_SUSPENDED)
		{
			int ret = script.context->Execute();
			if (ret == AngelScript::asEContextState::asEXECUTION_EXCEPTION)
			{
				Themp::Print("Script %s has run into an exception! [Function: %s] [Line: %i] [Exception: %s]", script.scriptName.c_str(), script.context->GetExceptionFunction()->GetName(), script.context->GetExceptionLineNumber(), script.context->GetExceptionString());
				Themp::Break();
			}
			else if (ret == AngelScript::asEContextState::asEXECUTION_ERROR)
			{
				Themp::Print("Script %s has run into an error! [%s]", script.scriptName.c_str(), script.context->GetExceptionString());
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
							Themp::Print("Coroutine in script %s has run into an exception! [Function: %s] [Line: %i] [Exception: %s]", script.scriptName.c_str(), coroutine->GetExceptionFunction()->GetName(), coroutine->GetExceptionLineNumber(), coroutine->GetExceptionString());
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
				for (int i = static_cast<int>(script.coroutines.size()) - 1; i >= 0; i--)
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
