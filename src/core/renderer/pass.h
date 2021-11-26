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
		struct DepthWriteMask
		{
			enum Mask
			{
				ZERO,
				ALL,
				COUNT
			};
			constexpr static std::string_view Str[COUNT]
			{
				"zero",
				"all",
			};
			Mask mask;
		};
		struct ComparisonFunc
		{
			enum Func
			{
				NEVER,
				LESS,
				EQUAL,
				LESS_EQUAL,
				GREATER,
				NOT_EQUAL,
				GREATER_EQUAL,
				ALWAYS,
				COUNT
			};
			constexpr static std::string_view Str[COUNT]
			{
				"never",
				"less",
				"equal",
				"less_equal",
				"greater",
				"not_equal",
				"greater_equal",
				"always"
			};
			Func func;
		};
		struct StencilOp
		{
			enum Op
			{
				KEEP,
				ZERO,
				REPLACE,
				INCR_SAT,
				DECR_SAT,
				INVERT,
				INCR,
				DECR,
				COUNT
			};
			constexpr static std::string_view Str[COUNT]
			{
				"keep",
				"zero",
				"replace",
				"incr_sat",
				"decr_sat",
				"invert",
				"incr",
				"decr"
			};
			Op op;
		};
		struct FillMode
		{
			enum Mode
			{
				WIREFRAME,
				SOLID,
				COUNT
			};
			constexpr static std::string_view Str[COUNT]
			{
				"wireframe",
				"solid"
			};
			Mode mode;
		};
		struct CullMode
		{
			enum Mode
			{
				NONE,
				FRONT,
				BACK,
				COUNT
			};
			constexpr static std::string_view Str[COUNT]
			{
				"none",
				"front",
				"back"
			};
			Mode mode;
		};
		public:
		struct Blend
		{
			enum BlendType
			{
				ZERO,
				ONE,
				SRC_COLOR,
				INV_SRC_COLOR,
				SRC_ALPHA,
				INV_SRC_ALPHA,
				DEST_ALPHA,
				INV_DEST_ALPHA,
				DEST_COLOR,
				INV_DEST_COLOR,
				SRC_ALPHA_SAT,
				BLEND_FACTOR,
				INV_BLEND_FACTOR,
				SRC1_COLOR,
				INV_SRC1_COLOR,
				SRC1_ALPHA,
				INV_SRC1_ALPHA,
				COUNT
			};
			constexpr static std::string_view Str[COUNT]
			{
				"zero",
				"one",
				"src_color",
				"inv_src_color",
				"src_alpha",
				"inv_src_alpha",
				"dest_alpha",
				"inv_dest_alpha",
				"dest_color",
				"inv_dest_color",
				"src_alpha_sat",
				"blend_factor",
				"inv_blend_factor",
				"src1_color",
				"inv_src1_color",
				"src1_alpha",
				"inv_src1_alpha"
			};

			void SetFromString(std::string_view str)
			{
				for (int i = 0; i < COUNT; i++)
				{
					if (str == Str[i])
					{
						type = (BlendType)i;
						break;
					}
				}
			}

			BlendType type;
		};
		struct BlendOp
		{
			enum Op
			{
				ADD,
				SUBTRACT,
				REV_SUBTRACT,
				MIN,
				MAX,
				COUNT
			};
			constexpr static std::string_view Str[COUNT]
			{
				"add",
				"substract",
				"rev_substract",
				"min",
				"max",
			};
			void SetFromString(std::string_view str)
			{
				for (int i = 0; i < COUNT; i++)
				{
					if (str == Str[i])
					{
						op = (Op)i;
						break;
					}
				}
			}
			Op op;
		};
		struct LogicOp
		{
			enum Op {
				CLEAR,
				SET,
				COPY,
				COPY_INVERTED,
				NOOP,
				INVERT,
				AND,
				NAND,
				OR,
				NOR,
				XOR,
				EQUIV,
				AND_REVERSE,
				AND_INVERTED,
				OR_REVERSE,
				OR_INVERTED,
				COUNT
			};

			constexpr static std::string_view Str[COUNT]
			{
				"clear",
				"set",
				"copy",
				"copy_inverted",
				"noop",
				"invert",
				"and",
				"nand",
				"or ",
				"nor",
				"xor",
				"equiv",
				"and_reverse",
				"and_inverted",
				"or_reverse",
				"or_inverted"
			};

			void SetFromString(std::string_view str)
			{
				for (int i = 0; i < COUNT; i++)
				{
					if (str == Str[i])
					{
						op = (Op)i;
						break;
					}
				}
			}
			Op op;
		};

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
		private:
		struct RasterState
		{
			FillMode fillMode;
			CullMode cullMode;
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
			DepthBias,
			DepthBiasClamp,
			SlopeScaledDepthBias,
			DepthClipEnable,
			MultisampleEnable,
			AntialiasedLineEnable,
			ForcedSampleCount,
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
			"depthbias",
			"depthbiasclamp",
			"slopescaleddepthbias",
			"depthclipenable",
			"multisampleenable",
			"antialiasedlineenable",
			"forcedsamplecount",
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
		void SetDepthBias(int bias);
		void SetDepthBiasClamp(float clamp);
		void SetSlopeScaledDepthBias(float bias);
		void SetDepthClipEnable(bool enabled);
		void SetMultisampleEnable(bool enabled);
		void SetAntialiasedLineEnable(bool enabled);
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
		int m_MultisampleCount;
		int m_MultisampleQuality;
		std::array<BlendState, 8> m_BlendStates;
		std::array<RenderTargetHandle, 8> m_RenderTargets;
	protected:

		friend class Control;
	};
	
}