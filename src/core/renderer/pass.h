#pragma once

#include <vector>
#include <array>
#include "renderer/types.h"
#include <string>

namespace Themp::D3D
{
	class Pipeline;
	class Pass
	{
	public:
		struct BlendState
		{
			bool blendEnable;
			bool logicOpEnable;
			Blend srcBlend;
			Blend destBlend;
			BlendOp blendOp;
			Blend srcBlendAlpha;
			Blend destBlendAlpha;
			BlendOp blendAlphaOp;
			LogicOp logicOp;
			uint8_t	renderTargetWriteMask;
		};
		struct RasterState
		{
			FillMode fillMode;
			CullMode cullMode;
			ConservativeRaster conservativeRaster;
			bool frontCounterClockwise;
			int depthBias;
			float depthBiasClamp;
			float slopeScaledDepthBias;
			bool depthClipEnable;
			bool multisampleEnable;
			bool antialiasedLineEnable;
			unsigned int forcedSampleCount;
			bool alphaToCoverageEnable;
			bool independendBlendEnable;
		};

		struct DepthState
		{
			struct StencilDesc
			{
				StencilOp stencilFailOp;
				StencilOp stencilDepthFailOp;
				StencilOp stencilPassOp;
				ComparisonFunc stencilFunc;
			};
			bool depthEnable;
			DepthWriteMask depthWriteMask;
			ComparisonFunc depthFunc;
			unsigned int sampleMask;
			bool stencilEnable;
			uint8_t stencilReadMask;
			uint8_t stencilWriteMask;
			StencilDesc frontFace;
			StencilDesc backFace;
		};

		struct RenderTargetSlot
		{
			RenderTargetHandle handle;
			int slot;
		};

	public:
		enum class Member
		{
			//determines the order of rendering
			Priority,
			//depth state
			DepthEnable,
			DepthWriteMask,
			DepthFunc,
			StencilEnable,
			StencilReadMask,
			StencilWriteMask,
			//stencil
			FrontFaceFailOp,
			FrontFaceDepthFailOp,
			FrontFacePassOp,
			FrontFaceFunc,
			BackFaceFailOp,
			BackFaceDepthFailOp,
			BackFacePassOp,
			BackFaceFunc,
			//raster
			FrontCounterClockwise,
			FillMode,
			CullMode,
			ConservativeRaster,
			DepthBias,
			DepthBiasClamp,
			SlopeScaledDepthBias,
			DepthClipEnable,
			MultisampleEnable,
			AntialiasedLineEnable,
			ForcedSampleCount,
			Topology,
			//global blend settings
			AlphaToCoverage,
			IndependendBlend,
			SampleMask,
			//per target blend state
			BlendEnable,
			RenderTargetIndex,
			SrcBlend,
			DestBlend,
			BlendOp,
			SrcAlphaBlend,
			DestAlphaBlend,
			BlendAlphaOp,
			LogicOpEnable,
			LogicOp,
			RenderTargetWriteMask,
			//multisampling
			MultisampleCount,
			MultisampleQuality,
			//targets
			DepthTarget, //string
			ColorTarget, //string
			BlendTargets, //string
			COUNT
		};

	private:
		static constexpr std::string_view PassMembersStr[static_cast<size_t>(Member::COUNT)]
		{
			"priority",
			"depthenable",
			"depthwritemask",
			"depthfunc",
			"stencilenable",
			"stencilreadmask",
			"stencilwritemask",
			"frontfacefailop",
			"frontfacedepthfailop",
			"frontfacepassop",
			"frontfacefunc",
			"backfacefailop",
			"backfacedepthfailop",
			"backfacepassop",
			"backfacefunc",
			"frontcounterclockwise",
			"fillmode",
			"cullmode",
			"conservativeraster",
			"depthbias",
			"depthbiasclamp",
			"slopescaleddepthbias",
			"depthclipenable",
			"multisampleenable",
			"antialiasedlineenable",
			"forcedsamplecount",
			"topology",
			"alphatocoverage",
			"independendblend",
			"samplemask",
			"blendenable",
			"rendertargetindex",
			"srcblend",
			"destblend",
			"blendop",
			"srcalphablend",
			"destalphablend",
			"blendalphaop",
			"logicopenable",
			"logicop",
			"rendertargetwritemask",
			"multisamplecount",
			"multisamplequality",
			"depthtarget",
			"colortarget",
			"blendtargets",
		};

	public:

		static std::string_view GetPassMemberAsString(Member member) { return PassMembersStr[static_cast<size_t>(member)]; }


		explicit Pass(std::string_view name);
		std::string_view GetName() const;

		void SetPriority(int priority);
		void SetDepthEnable(bool enabled);
		void SetDepthWriteMask(std::string_view str);
		void SetDepthFunc(std::string_view str);
		void SetStencilEnable(bool enabled);
		void SetStencilReadMask(uint8_t mask);
		void SetStencilWriteMask(uint8_t mask);
		void SetFrontFaceFailOp(std::string_view str);
		void SetFrontFaceDepthFailOp(std::string_view str);
		void SetFrontFacePassOp(std::string_view str);
		void SetFrontFaceFunc(std::string_view str);
		void SetBackFaceFailOp(std::string_view str);
		void SetBackFaceDepthFailOp(std::string_view str);
		void SetBackFacePassOp(std::string_view str);
		void SetBackFaceFunc(std::string_view str);
		void SetFrontCounterClockwise(bool val);
		void SetFillMode(std::string_view str);
		void SetCullMode(std::string_view str);
		void SetConservativeRaster(std::string_view str);
		void SetDepthBias(int bias);
		void SetDepthBiasClamp(float clamp);
		void SetSlopeScaledDepthBias(float bias);
		void SetDepthClipEnable(bool enabled);
		void SetMultisampleEnable(bool enabled);
		void SetAntialiasedLineEnable(bool enabled);
		void SetTopology(std::string_view str);
		void SetForcedSampleCount(unsigned int count);
		void SetAlphaToCoverageEnable(bool enabled);
		void SetIndependendBlendEnable(bool enabled);
		void SetSampleMask(uint32_t mask);
		void SetMultisampleCount(int count);
		void SetMultisampleQuality(int quality);
		void SetDepthTarget(RenderTargetHandle handle);
		void SetColorTarget(int index, RenderTargetHandle handle);
		void SetBlendTarget(int index, const BlendState& state);


		void SetName(std::string_view name);
		bool IsValid() const;
	private:
		std::string m_Name;
		int m_Priority;
		DepthState m_DepthState;
		RasterState m_RasterState;
		RenderTargetHandle m_DepthTarget;
		unsigned int m_SampleMask;
		PrimitiveTopology m_Topology;
		int m_MultisampleCount;
		int m_MultisampleQuality;
		std::array<BlendState, 8> m_BlendStates;
		std::array<RenderTargetHandle, 8> m_RenderTargets;
	protected:

		friend class Control;
		friend class Pipeline;
		friend struct DxTranslater;
	};
	
}