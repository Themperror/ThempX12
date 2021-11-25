#pragma once

#include <vector>
#include "renderer/types.h"
#include <string>

namespace Themp::D3D
{
	class Pipeline;
	class Context;
	class Pass
	{
		struct RenderList
		{
			Pipeline* pipeline;
			std::vector<MeshData> m_Renderables;
		};
		

	public:
		enum class PassConfigurationMembers
		{
			PRIORITY, //int
			DEPTH_ENABLE, //bool
			DEPTH_WRITE, //bool
			DEPTH_FUNC, //enum
			WIREFRAME, //bool
			WINDINGORDER, //enum
			DEPTHTARGET, //string
			COLORTARGET, //string
			COUNT
		};

	private:
		static constexpr std::string_view PassConfigurationMembersStr[static_cast<size_t>(PassConfigurationMembers::COUNT)]
		{
			"priority",
			"depth_enable",
			"depth_write",
			"depth_func",
			"wireframe",
			"windingorder",
			"depthtarget",
			"colortarget",
		};

	public:

		static std::string_view GetPassConfigurationMembersAsString(PassConfigurationMembers member) { return PassConfigurationMembersStr[static_cast<size_t>(member)]; }


		explicit Pass(std::string_view name);
		void AddRenderable(ShaderHandle material);
		void AddRenderable(ShaderHandle material, const MeshData& mesh);

		void Draw(Context& context);
		void SetPriority(int priority);
		void SetName(std::string_view name);
		bool IsValid() { return m_Name.size() > 0 && m_Priority != std::numeric_limits<int>::lowest(); }
	private:
		std::vector<RenderList> m_RenderList;
		std::string m_Name;
		int m_Priority;
	protected:

		friend class Control;
	};
	
}