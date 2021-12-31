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


	D3D::ConstantBufferHandle GetConstantBuffer()
	{
		return Themp::Engine::instance->m_Renderer->GetResourceManager().ReserveConstantBuffer();
	}

	void DeclareConstantBufferMemberFloat(D3D::ConstantBufferHandle& handle, const std::string& name)
	{
		auto& CB = Themp::Engine::instance->m_Renderer->GetResourceManager().Get(handle);
		auto it = CB.memberData.find(name);
		if (it == CB.memberData.end())
		{
			int32_t nextOffset = CB.nextOffset;
			CB.memberData[name] = std::pair(nextOffset, "float");
			CB.nextOffset += sizeof(float);
			CB.data.resize(CB.nextOffset);
		}
		else
		{
			Themp::Print("Tried to declare member: %s as float, but it already exists on index %llu with type %*s", name.c_str(), it->second.first, it->second.second.size(), it->second.second.data());
		}
	}

	void DeclareConstantBufferMemberFloat2(D3D::ConstantBufferHandle& handle, const std::string& name)
	{
		auto& CB = Themp::Engine::instance->m_Renderer->GetResourceManager().Get(handle);
		constexpr int64_t float2Size = sizeof(float) * 2;
		auto it = CB.memberData.find(name);
		if (it == CB.memberData.end())
		{
			int32_t nextOffset = CB.nextOffset;

			if ((nextOffset % 16) + float2Size > 16)
			{
				CB.nextOffset += 16 - (nextOffset % 16); //skip whatever is needed to align on 16 bytes
			}
			CB.memberData[name] = std::pair(CB.nextOffset, "float2");
			CB.nextOffset += float2Size;
			CB.data.resize(CB.nextOffset);
		}
		else
		{
			Themp::Print("Tried to declare member: %s as float2, but it already exists on index %llu with type %*s", name.c_str(), it->second.first, it->second.second.size(), it->second.second.data());
		}
	}

	template <typename T>
	void SetConstantBufferData(D3D::ConstantBufferHandle& handle, const std::string& name, T value)
	{
		auto& CB = Themp::Engine::instance->m_Renderer->GetResourceManager().Get(handle);
		
		const auto& it = CB.memberData.find(name);
		if (it != CB.memberData.end())
		{
			size_t offset = it->second.first;
			if (memcmp(CB.data.data() + offset, &value, sizeof(T)) != 0)
			{
				memcpy(CB.data.data() + offset, &value, sizeof(T));
				CB.dirty = true;
			}
		}
		else
		{
			Themp::Print("Tried to set member %s on constantbuffer %llu, but it was never declared", name.c_str(), handle.handle);
			Themp::Break();
		}
	}

	void SetConstantBuffer(int slot, CBufferType val)
	{
		Themp::Print("Set constantbuffer slot %i with system buffer: %s", slot, val == Camera ? "Camera" : "Engine");

		if (val == CBufferType::Empty)
		{
			std::pair<int, D3D::ConstantBufferHandle>* constantBuffer = nullptr;
			for (int i = 0; i < currentRenderPass->constantBuffers.size(); i++)
			{
				auto& CB = currentRenderPass->constantBuffers[i];
				if (CB.first == slot)
				{
					currentRenderPass->constantBuffers.erase(currentRenderPass->constantBuffers.begin() + i);
					return;
				}
			}
		}
		else
		{
			D3D::ConstantBufferHandle handle = val == CBufferType::Engine ? Themp::Engine::instance->m_Renderer->GetEngineConstantBuffer() : Themp::Engine::instance->m_Renderer->GetCameraConstantBuffer();

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

	float GetTime()
	{
		return Themp::Engine::instance->GetTimeSinceLaunch();
	}
	float GetDeltaTime()
	{
		return Themp::Engine::instance->GetDeltaTime();
	}

	void Registrar::Init(AngelScript::asIScriptEngine* scriptingEngine)
	{
		RegisterGlobalFunctions(scriptingEngine);
		RegisterMemberFunctions(scriptingEngine);
	}


	void Registrar::RegisterGlobalFunctions(AngelScript::asIScriptEngine* scriptingEngine)
	{
		CheckResult(scriptingEngine->RegisterGlobalFunction("void Print(const string &in)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&), void), AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void Print(const string &in, int param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, int), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void Print(const string &in, float param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, float), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void Print(const string &in, string param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, std::string&), void) , AngelScript::asCALL_CDECL), ASPrint);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void Print(const string &in, uint64 param0)", AngelScript::asFUNCTIONPR(ASPrint, (const std::string&, uint64_t), void) , AngelScript::asCALL_CDECL), ASPrint);

		CheckResult(scriptingEngine->RegisterGlobalFunction("float GetTime()", AngelScript::asFUNCTIONPR(GetTime, (void), float) , AngelScript::asCALL_CDECL), GetTime);
		CheckResult(scriptingEngine->RegisterGlobalFunction("float GetDeltaTime()", AngelScript::asFUNCTIONPR(GetDeltaTime, (void), float) , AngelScript::asCALL_CDECL), GetDeltaTime);
		
		RegisterConstantBufferHandle(scriptingEngine);
		RegisterRenderPassHandle(scriptingEngine);

		scriptingEngine->RegisterEnum("CBufferType");
		scriptingEngine->RegisterEnumValue("CBufferType", "Empty", 0);
		scriptingEngine->RegisterEnumValue("CBufferType", "Camera", 1);
		scriptingEngine->RegisterEnumValue("CBufferType", "Engine", 2);

		scriptingEngine->RegisterObjectType("float2", sizeof(float) * 2, AngelScript::asEObjTypeFlags::asOBJ_VALUE | AngelScript::asEObjTypeFlags::asOBJ_APP_CLASS_ALLFLOATS | AngelScript::asEObjTypeFlags::asOBJ_POD | AngelScript::asGetTypeTraits<DirectX::XMFLOAT2>());
		scriptingEngine->RegisterObjectProperty("float2", "float x", 0);
		scriptingEngine->RegisterObjectProperty("float2", "float y", sizeof(float));
		scriptingEngine->RegisterObjectProperty("float2", "float r", 0);
		scriptingEngine->RegisterObjectProperty("float2", "float g", sizeof(float));

		CheckResult(scriptingEngine->RegisterGlobalFunction("ConstantBufferHandle GetConstantBuffer ()", AngelScript::asFUNCTIONPR(GetConstantBuffer, (void), D3D::ConstantBufferHandle) , AngelScript::asCALL_CDECL), GetConstantBuffer);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBuffer (int slot, CBufferType val)", AngelScript::asFUNCTIONPR(SetConstantBuffer, (int slot, CBufferType val), void) , AngelScript::asCALL_CDECL), SetConstantBuffer);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBuffer (int slot, ConstantBufferHandle &in)", AngelScript::asFUNCTIONPR(SetConstantBuffer, (int slot, D3D::ConstantBufferHandle& val), void) , AngelScript::asCALL_CDECL), SetConstantBuffer);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBufferData (ConstantBufferHandle &in, string &in, float val)", AngelScript::asFUNCTIONPR(SetConstantBufferData<float>, (D3D::ConstantBufferHandle&, const std::string&, float), void) , AngelScript::asCALL_CDECL), SetConstantBufferData<float>);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBufferData (ConstantBufferHandle &in, string &in, int val)", AngelScript::asFUNCTIONPR(SetConstantBufferData<int>, (D3D::ConstantBufferHandle&, const std::string&, int), void) , AngelScript::asCALL_CDECL), SetConstantBufferData<int>);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void SetConstantBufferData (ConstantBufferHandle &in, string &in, float2 val)", AngelScript::asFUNCTIONPR(SetConstantBufferData<DirectX::XMFLOAT2>, (D3D::ConstantBufferHandle&, const std::string&, DirectX::XMFLOAT2), void) , AngelScript::asCALL_CDECL), SetConstantBufferData<DirectX::XMFLOAT2>);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void DeclareConstantBufferMemberFloat (ConstantBufferHandle &in, string &in)", AngelScript::asFUNCTIONPR(DeclareConstantBufferMemberFloat, (D3D::ConstantBufferHandle&, const std::string&), void) , AngelScript::asCALL_CDECL), DeclareConstantBufferMemberFloat);
		CheckResult(scriptingEngine->RegisterGlobalFunction("void DeclareConstantBufferMemberFloat2 (ConstantBufferHandle &in, string &in)", AngelScript::asFUNCTIONPR(DeclareConstantBufferMemberFloat2, (D3D::ConstantBufferHandle&, const std::string&), void) , AngelScript::asCALL_CDECL), DeclareConstantBufferMemberFloat2);
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