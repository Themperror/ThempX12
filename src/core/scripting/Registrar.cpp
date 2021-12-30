#include "Registrar.h"
#include "core/util/print.h"
#include "core/util/break.h"
#include "core/renderer/types.h"

#include "core/engine.h"
#include "core/renderer/gpu_resources.h"
#include "core/renderer/control.h"


#include <string>
#include <lib/angelscript/angelscript.h>
#include <lib/angelscript/add_on/scripthandle/scripthandle.h>

#define CheckResult(result, funcName) \
	if ((result) < 0) { \
	Themp::Print("Failed to register" #funcName " to ASEngine"); \
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

	void ASPrint(const std::string& msg, uint64_t param0)
	{
		Themp::Print(msg, param0);
	}

	enum CBufferType
	{
		Empty,
		Camera,
		Engine,
	};


	Themp::D3D::RenderPass* currentRenderPass = nullptr;
	void Registrar::SetCurrentRenderPass(D3D::RenderPassHandle handle)
	{
		currentRenderPass = &Themp::Engine::instance->m_Renderer->GetRenderPass(handle);
	}


	D3D::ConstantBufferHandle GetConstantBuffer(uint64_t size)
	{
		return Themp::Engine::instance->m_Renderer->GetResourceManager().CreateConstantBuffer(Themp::Engine::instance->m_Renderer->GetDevice(), size);
	}

	void SetConstantBufferData(D3D::ConstantBufferHandle& handle, uint32_t offset, float value)
	{
		auto& CB = Themp::Engine::instance->m_Renderer->GetResourceManager().Get(handle);
		if (offset + sizeof(float) > CB.data.size())
		{
			Themp::Break();
			return;
		}

		float origValue = 0;
		memcpy(&origValue, CB.data.data() + offset, sizeof(value));
		if (origValue != value)
		{
			memcpy(CB.data.data() + offset, &value, sizeof(value));
			CB.dirty = true;
		}
	}

	void SetConstantBuffer(int slot, CBufferType val)
	{
		Themp::Print("Set constantbuffer slot %i with system buffer: %s", slot, val == Camera ? "Camera" : "Engine");
	}
	
	void SetConstantBuffer(int slot, D3D::ConstantBufferHandle& handle)
	{
		Themp::Print("Set constantbuffer slot %i with handle: %llu", slot, handle.handle);
		for (auto& CB : currentRenderPass->constantBuffers)
		{
			if (CB.first == slot)
			{
				CB.second = handle;
				return;
			}
		}
		currentRenderPass->constantBuffers.emplace_back(std::pair(slot, handle));
	}

	void Registrar::Init(AngelScript::asIScriptEngine* scriptingEngine)
	{
		RegisterGlobalFunctions(scriptingEngine);
		RegisterMemberFunctions(scriptingEngine);
	}

#define CreateHandleRegistrar(name_space, x)  \
static void x##New(void* mem) { auto* m = new(mem) name_space::x(); } \
static void x##New(name_space::x rhs, name_space::x* memory) { auto* m = new(memory) name_space::x(); memory->handle = rhs.handle; } \
static void x##Assignment(const name_space::x& rhs, name_space::x& lhs) { lhs = rhs;} \
static void x##Destructor(void* memory) { name_space::x* handle = ((name_space::x*)memory); handle->~x(); } \
void Register##x(AngelScript::asIScriptEngine* scriptingEngine) { \
	CheckResult(scriptingEngine->RegisterObjectType(#x, sizeof(name_space::x), AngelScript::asEObjTypeFlags::asOBJ_VALUE | AngelScript::asGetTypeTraits<name_space::x>()), "RegisterObjectType("#x")"); \
	CheckResult(scriptingEngine->RegisterObjectProperty(#x, "uint64 handle", 0), "RegisterObjectProperty(uint64 handle)"); \
	CheckResult(scriptingEngine->RegisterObjectBehaviour(#x, AngelScript::asBEHAVE_CONSTRUCT, "void "#x"Constructor()", AngelScript::asFUNCTIONPR(x##New,(void*),void), AngelScript::asCALL_CDECL_OBJLAST), #x"constructMem");\
	CheckResult(scriptingEngine->RegisterObjectBehaviour(#x, AngelScript::asBEHAVE_CONSTRUCT, "void "#x"Constructor("#x")", AngelScript::asFUNCTIONPR(x##New,(name_space::x rhs, name_space::x* memory),void), AngelScript::asCALL_CDECL_OBJLAST), #x"constructCopy");\
	CheckResult(scriptingEngine->RegisterObjectBehaviour(#x, AngelScript::asBEHAVE_DESTRUCT, "void "#x"Destructor()", AngelScript::asFUNCTIONPR(x##Destructor,(void* memory),void), AngelScript::asCALL_CDECL_OBJLAST), #x"destruct"); \
	CheckResult(scriptingEngine->RegisterObjectMethod(#x, #x"& opAssign(const "#x" &in other)", AngelScript::asFUNCTIONPR(x##Assignment, (const name_space::x&, name_space::x&), void), AngelScript::asCALL_CDECL_OBJLAST), #x"assign"); \
} 


	CreateHandleRegistrar(D3D, ConstantBufferHandle)
	CreateHandleRegistrar(D3D, RenderPassHandle)



	void Registrar::RegisterGlobalFunctions(AngelScript::asIScriptEngine* scriptingEngine)
	{
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&), void), AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in, int param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, int), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in, float param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, float), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in, string param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, std::string&), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void print(const string &in, uint64 param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, uint64_t), void) , AngelScript::asCALL_CDECL), ASPrint);
		
		RegisterConstantBufferHandle(scriptingEngine);
		RegisterRenderPassHandle(scriptingEngine);

		scriptingEngine->RegisterEnum("CBufferType");
		scriptingEngine->RegisterEnumValue("CBufferType", "Empty", 0);
		scriptingEngine->RegisterEnumValue("CBufferType", "Camera", 1);
		scriptingEngine->RegisterEnumValue("CBufferType", "Engine", 2);
		
		CheckResult(scriptingEngine->RegisterGlobalFunction("ConstantBufferHandle GetConstantBuffer (uint64 size)", AngelScript::asFUNCTIONPR(GetConstantBuffer, (uint64_t), D3D::ConstantBufferHandle) , AngelScript::asCALL_CDECL), GetConstantBuffer);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBuffer (int slot, CBufferType val)", AngelScript::asFUNCTIONPR(SetConstantBuffer, (int slot, CBufferType val), void) , AngelScript::asCALL_CDECL), SetConstantBuffer);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBuffer (int slot, ConstantBufferHandle &in)", AngelScript::asFUNCTIONPR(SetConstantBuffer, (int slot, D3D::ConstantBufferHandle& val), void) , AngelScript::asCALL_CDECL), SetConstantBuffer);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBufferData (ConstantBufferHandle &in, uint offset, float val)", AngelScript::asFUNCTIONPR(SetConstantBufferData, (D3D::ConstantBufferHandle& handle, uint32_t offset, float val), void) , AngelScript::asCALL_CDECL), SetConstantBufferData);
	}

	void Registrar::RegisterMemberFunctions(AngelScript::asIScriptEngine* scriptingEngine)
	{

		//int r = scriptingEngine->RegisterGlobalProperty("RenderPassHandle", sizeof(Themp::D3D::RenderPassHandle), AngelScript::asEObjTypeFlags::asOBJ_APP_PRIMITIVE | AngelScript::asEObjTypeFlags::asOBJ_POD |  AngelScript::asEObjTypeFlags::asOBJ_VALUE);
		//if (r < 0)
		//{
		//	Themp::Break();
		//}

	}
}