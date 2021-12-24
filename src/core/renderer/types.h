#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include "core/util/print.h"
#include <cstdint>
#include <limits>
using namespace Microsoft::WRL;

//fix conflics with std::numeric_limits<size_t>::max(), thanks windows.h (even though I defined NOMINMAX)
#ifdef max
#undef max
#endif

#define Handle(x) struct x##Handle { \
size_t handle = Invalid; \
x##Handle() = default; \
x##Handle(size_t rhs) noexcept { handle = rhs; } \
x##Handle(const x##Handle& rhs) noexcept{ this->handle = rhs.handle;} \
x##Handle(x##Handle&& rhs) noexcept { handle = rhs.handle;} \
x##Handle& operator=(const x##Handle& rhs) noexcept{ this->handle = rhs.handle; return *this;} \
x##Handle& operator=(x##Handle&& rhs) noexcept { this->handle = rhs.handle; return *this;} \
bool operator==(const size_t rhs) noexcept { return handle == rhs; }\
bool operator==(const x##Handle& rhs) noexcept { return handle == rhs.handle; }\
bool IsValid() const noexcept { return handle != Invalid;} \
static inline constexpr const size_t Invalid = std::numeric_limits<size_t>::max(); }

namespace Themp::D3D
{
	//struct PassHandle
	//{
	//	size_t handle = Invalid;
	//	constexpr PassHandle() = default;
	//	constexpr PassHandle(size_t val) { handle = val; }
	//	constexpr PassHandle(PassHandle& val) { handle = val.handle; }
	//	constexpr PassHandle(PassHandle&& val) noexcept { handle = val.handle; }
	//	PassHandle operator=(const PassHandle& rhs) { return { rhs.handle }; }
	//	PassHandle& operator=(PassHandle&& rhs)  noexcept { return rhs; }
	//	PassHandle operator=(const size_t rhs) { return { rhs }; }
	//	bool operator==(const size_t rhs) { return handle == rhs; }
	//	bool operator==(const PassHandle& rhs) { return handle == rhs.handle; }
	//	bool IsValid() const { return handle != Invalid; }
	//	static inline constexpr const size_t Invalid = std::numeric_limits<size_t>::max();
	//};

	Handle(Pass);
	Handle(SubPass);
	Handle(Material);
	Handle(Shader);
	Handle(RTV);
	Handle(DSV);
	Handle(SRV);
	Handle(Texture);
	Handle(Model);

	struct RenderTargetHandle
	{
		RenderTargetHandle() = default; 
		RenderTargetHandle(RTVHandle rtvHandle, DSVHandle dsvHandle, SRVHandle srvHandle) 
		{
			rtv = rtvHandle;
			srv = srvHandle;
			dsv = dsvHandle;
		}
		RenderTargetHandle(const RenderTargetHandle& rhs) noexcept 
		{
			rtv = rhs.rtv;
			srv = rhs.srv;
			dsv = rhs.dsv;
		} 
		RenderTargetHandle(RenderTargetHandle&& rhs) noexcept 
		{
			rtv = rhs.rtv;
			srv = rhs.srv;
			dsv = rhs.dsv;
		} 
		RenderTargetHandle& operator=(const RenderTargetHandle& rhs) noexcept
		{ 
			rtv = rhs.rtv;
			srv = rhs.srv;
			dsv = rhs.dsv;
			return *this;
		} 
		RenderTargetHandle& operator=(RenderTargetHandle&& rhs) noexcept 
		{
			rtv = rhs.rtv;
			srv = rhs.srv;
			dsv = rhs.dsv; 
			return *this;
		} 

		RTVHandle rtv = RTVHandle::Invalid;
		SRVHandle srv = SRVHandle::Invalid;
		DSVHandle dsv = DSVHandle::Invalid;

		static RenderTargetHandle Invalid() { return {}; };
	};

	struct SubPass
	{
		bool NeedsPositionInfo;
		bool NeedsNormalInfo;
		bool NeedsUVInfo;
		PassHandle pass = PassHandle::Invalid;
		ShaderHandle shader = ShaderHandle::Invalid;
	};


	enum class DESCRIPTOR_HEAP_TYPE
	{
		CB_SRV_UAV,
		SAMPLER,
		RTV,
		DSV,
	};
	enum class TEXTURE_TYPE
	{
		SRV,
		UAV,
		RTV,
		DSV,
	};


	struct DescriptorHeapTracker
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
		uint32_t usedSlots = 0;
		uint32_t maxSlots = 0;
	};

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
		DirectX::XMFLOAT2 uv;
	};


	struct MeshData
	{
		uint32_t vertexIndex;
		uint32_t vertexCount;
		uint32_t indexIndex;
		uint32_t indexCount;
	};

	struct ModelData
	{
		std::vector<MeshData> meshes;
	};

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

	struct PrimitiveTopology
	{
		enum Primitive
		{
			POINT,
			LINE,
			TRIANGLE,
			PATCH,
			COUNT
		};
		constexpr static std::string_view Str[COUNT]
		{
			"point",
			"line",
			"triangle",
			"patch",
		};
		Primitive primitive;
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
	struct ConservativeRaster
	{
		enum Mode
		{
			OFF,
			ON,
			COUNT
		};
		constexpr static std::string_view Str[COUNT]
		{
			"off",
			"on"
		};
		Mode mode;


		void SetFromString(std::string_view str)
		{
			for (int i = 0; i < COUNT; i++)
			{
				if (str == Str[i])
				{
					mode = (Mode)i;
					break;
				}
			}
		}
	};

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

	struct TextureFormat
	{
		enum Format
		{
			R32G32B32A32_TYPELESS,
			R32G32B32A32_FLOAT,
			R32G32B32A32_UINT,
			R32G32B32A32_SINT,
			R32G32B32_TYPELESS,
			R32G32B32_FLOAT,
			R32G32B32_UINT,
			R32G32B32_SINT,
			R16G16B16A16_TYPELESS,
			R16G16B16A16_FLOAT,
			R16G16B16A16_UNORM,
			R16G16B16A16_UINT,
			R16G16B16A16_SNORM,
			R16G16B16A16_SINT,
			R32G32_TYPELESS,
			R32G32_FLOAT,
			R32G32_UINT,
			R32G32_SINT,
			R32G8X24_TYPELESS,
			D32_FLOAT_S8X24_UINT,
			R32_FLOAT_X8X24_TYPELESS,
			X32_TYPELESS_G8X24_UINT,
			R10G10B10A2_TYPELESS,
			R10G10B10A2_UNORM,
			R10G10B10A2_UINT,
			R11G11B10_FLOAT,
			R8G8B8A8_TYPELESS,
			R8G8B8A8_UNORM,
			R8G8B8A8_UNORM_SRGB,
			R8G8B8A8_UINT,
			R8G8B8A8_SNORM,
			R8G8B8A8_SINT,
			R16G16_TYPELESS,
			R16G16_FLOAT,
			R16G16_UNORM,
			R16G16_UINT,
			R16G16_SNORM,
			R16G16_SINT,
			R32_TYPELESS,
			D32_FLOAT,
			R32_FLOAT,
			R32_UINT,
			R32_SINT,
			R24G8_TYPELESS,
			D24_UNORM_S8_UINT,
			R24_UNORM_X8_TYPELESS,
			X24_TYPELESS_G8_UINT,
			R8G8_TYPELESS,
			R8G8_UNORM,
			R8G8_UINT,
			R8G8_SNORM,
			R8G8_SINT,
			R16_TYPELESS,
			R16_FLOAT,
			D16_UNORM,
			R16_UNORM,
			R16_UINT,
			R16_SNORM,
			R16_SINT,
			R8_TYPELESS,
			R8_UNORM,
			R8_UINT,
			R8_SNORM,
			R8_SINT,
			A8_UNORM,
			R1_UNORM,
			R9G9B9E5_SHAREDEXP,
			R8G8_B8G8_UNORM,
			G8R8_G8B8_UNORM,
			BC1_TYPELESS,
			BC1_UNORM,
			BC1_UNORM_SRGB,
			BC2_TYPELESS,
			BC2_UNORM,
			BC2_UNORM_SRGB,
			BC3_TYPELESS,
			BC3_UNORM,
			BC3_UNORM_SRGB,
			BC4_TYPELESS,
			BC4_UNORM,
			BC4_SNORM,
			BC5_TYPELESS,
			BC5_UNORM,
			BC5_SNORM,
			B5G6R5_UNORM,
			B5G5R5A1_UNORM,
			B8G8R8A8_UNORM,
			B8G8R8X8_UNORM,
			R10G10B10_XR_BIAS_A2_UNORM,
			B8G8R8A8_TYPELESS,
			B8G8R8A8_UNORM_SRGB,
			B8G8R8X8_TYPELESS,
			B8G8R8X8_UNORM_SRGB,
			BC6H_TYPELESS,
			BC6H_UF16,
			BC6H_SF16,
			BC7_TYPELESS,
			BC7_UNORM,
			BC7_UNORM_SRGB,
			AYUV,
			Y410,
			Y416,
			NV12,
			P010,
			P016,
			_420_OPAQUE,
			YUY2,
			Y210,
			Y216,
			NV11,
			AI44,
			IA44,
			P8,
			A8P8,
			B4G4R4A4_UNORM,
			P208,
			V208,
			V408,
			COUNT,
			INVALID = 0xFFFFFFFF
		};
		constexpr static std::string_view Str[COUNT]
		{
			"r32g32b32a32_typeless",
			"r32g32b32a32_float",
			"r32g32b32a32_uint",
			"r32g32b32a32_sint",
			"r32g32b32_typeless",
			"r32g32b32_float",
			"r32g32b32_uint",
			"r32g32b32_sint",
			"r16g16b16a16_typeless",
			"r16g16b16a16_float",
			"r16g16b16a16_unorm",
			"r16g16b16a16_uint",
			"r16g16b16a16_snorm",
			"r16g16b16a16_sint",
			"r32g32_typeless",
			"r32g32_float",
			"r32g32_uint",
			"r32g32_sint",
			"r32g8x24_typeless",
			"d32_float_s8x24_uint",
			"r32_float_x8x24_typeless",
			"x32_typeless_g8x24_uint",
			"r10g10b10a2_typeless",
			"r10g10b10a2_unorm",
			"r10g10b10a2_uint",
			"r11g11b10_float",
			"r8g8b8a8_typeless",
			"r8g8b8a8_unorm",
			"r8g8b8a8_unorm_srgb",
			"r8g8b8a8_uint",
			"r8g8b8a8_snorm",
			"r8g8b8a8_sint",
			"r16g16_typeless",
			"r16g16_float",
			"r16g16_unorm",
			"r16g16_uint",
			"r16g16_snorm",
			"r16g16_sint",
			"r32_typeless",
			"d32_float",
			"r32_float",
			"r32_uint",
			"r32_sint",
			"r24g8_typeless",
			"d24_unorm_s8_uint",
			"r24_unorm_x8_typeless",
			"x24_typeless_g8_uint",
			"r8g8_typeless",
			"r8g8_unorm",
			"r8g8_uint",
			"r8g8_snorm",
			"r8g8_sint",
			"r16_typeless",
			"r16_float",
			"d16_unorm",
			"r16_unorm",
			"r16_uint",
			"r16_snorm",
			"r16_sint",
			"r8_typeless",
			"r8_unorm",
			"r8_uint",
			"r8_snorm",
			"r8_sint",
			"a8_unorm",
			"r1_unorm",
			"r9g9b9e5_sharedexp",
			"r8g8_b8g8_unorm",
			"g8r8_g8b8_unorm",
			"bc1_typeless",
			"bc1_unorm",
			"bc1_unorm_srgb",
			"bc2_typeless",
			"bc2_unorm",
			"bc2_unorm_srgb",
			"bc3_typeless",
			"bc3_unorm",
			"bc3_unorm_srgb",
			"bc4_typeless",
			"bc4_unorm",
			"bc4_snorm",
			"bc5_typeless",
			"bc5_unorm",
			"bc5_snorm",
			"b5g6r5_unorm",
			"b5g5r5a1_unorm",
			"b8g8r8a8_unorm",
			"b8g8r8x8_unorm",
			"r10g10b10_xr_bias_a2_unorm",
			"b8g8r8a8_typeless",
			"b8g8r8a8_unorm_srgb",
			"b8g8r8x8_typeless",
			"b8g8r8x8_unorm_srgb",
			"bc6h_typeless",
			"bc6h_uf16",
			"bc6h_sf16",
			"bc7_typeless",
			"bc7_unorm",
			"bc7_unorm_srgb",
			"ayuv",
			"y410",
			"y416",
			"nv12",
			"p010",
			"p016",
			"420_opaque",
			"yuy2",
			"y210",
			"y216",
			"nv11",
			"ai44",
			"ia44",
			"p8",
			"a8p8",
			"b4g4r4a4_unorm",
			"p208",
			"v208",
			"v408",
		};
		void SetFromString(std::string_view str)
		{
			for (int i = 0; i < COUNT; i++)
			{
				if (str == Str[i])
				{
					format = (Format)i;
					return;
				}
			}
			Themp::Print("%*s was not a valid format!", str.size(), str.data());
			format = (Format)INVALID;
		}
		Format format;
	};

	struct DxTranslator
	{
		static D3D12_BLEND_OP GetBlendOp(BlendOp op)
		{
			switch (op.op)
			{
			default:
			case BlendOp::ADD:
				return D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
			case BlendOp::SUBTRACT:
				return D3D12_BLEND_OP::D3D12_BLEND_OP_SUBTRACT;
			case BlendOp::REV_SUBTRACT:
				return D3D12_BLEND_OP::D3D12_BLEND_OP_REV_SUBTRACT;
			case BlendOp::MIN:
				return D3D12_BLEND_OP::D3D12_BLEND_OP_MIN;
			case BlendOp::MAX:
				return D3D12_BLEND_OP::D3D12_BLEND_OP_MAX;
			}
		}

		static D3D12_LOGIC_OP GetLogicOp(LogicOp op)
		{
			switch (op.op)
			{
			default:
			case LogicOp::SET:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_SET;
			case LogicOp::AND:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_AND;
			case LogicOp::AND_INVERTED:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_AND_INVERTED;
			case LogicOp::AND_REVERSE:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_AND_REVERSE;
			case LogicOp::CLEAR:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_CLEAR;
			case LogicOp::COPY:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_COPY;
			case LogicOp::COPY_INVERTED:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_COPY_INVERTED;
			case LogicOp::EQUIV:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_EQUIV;
			case LogicOp::INVERT:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_INVERT;
			case LogicOp::NAND:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_NAND;
			case LogicOp::NOOP:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_NOOP;
			case LogicOp::NOR:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_NOR;
			case LogicOp::OR:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_OR;
			case LogicOp::OR_INVERTED:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_OR_INVERTED;
			case LogicOp::OR_REVERSE:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_OR_REVERSE;
			case LogicOp::XOR:
				return D3D12_LOGIC_OP::D3D12_LOGIC_OP_XOR;
			}
		}

		static D3D12_BLEND GetBlend(Blend op)
		{
			switch (op.type)
			{
			default:
			case Blend::ONE:
				return D3D12_BLEND::D3D12_BLEND_ONE;
			case Blend::ZERO:
				return D3D12_BLEND::D3D12_BLEND_ZERO;
			case Blend::BLEND_FACTOR:
				return D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR;
			case Blend::DEST_ALPHA:
				return D3D12_BLEND::D3D12_BLEND_DEST_ALPHA;
			case Blend::DEST_COLOR:
				return D3D12_BLEND::D3D12_BLEND_DEST_COLOR;
			case Blend::INV_BLEND_FACTOR:
				return D3D12_BLEND::D3D12_BLEND_INV_BLEND_FACTOR;
			case Blend::INV_DEST_ALPHA:
				return D3D12_BLEND::D3D12_BLEND_INV_DEST_ALPHA;
			case Blend::INV_DEST_COLOR:
				return D3D12_BLEND::D3D12_BLEND_INV_DEST_COLOR;
			case Blend::INV_SRC1_ALPHA:
				return D3D12_BLEND::D3D12_BLEND_INV_SRC1_ALPHA;
			case Blend::INV_SRC1_COLOR:
				return D3D12_BLEND::D3D12_BLEND_INV_SRC1_COLOR;
			case Blend::INV_SRC_ALPHA:
				return D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
			case Blend::INV_SRC_COLOR:
				return D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR;
			case Blend::SRC1_ALPHA:
				return D3D12_BLEND::D3D12_BLEND_SRC1_ALPHA;
			case Blend::SRC1_COLOR:
				return D3D12_BLEND::D3D12_BLEND_SRC1_COLOR;
			case Blend::SRC_ALPHA:
				return D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
			case Blend::SRC_ALPHA_SAT:
				return D3D12_BLEND::D3D12_BLEND_SRC_ALPHA_SAT;
			case Blend::SRC_COLOR:
				return D3D12_BLEND::D3D12_BLEND_SRC_COLOR;
			}
		}

		static D3D12_STENCIL_OP GetStencilOp(StencilOp op)
		{
			switch (op.op)
			{
			default:
			case StencilOp::KEEP:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
			case StencilOp::DECR:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR;
			case StencilOp::DECR_SAT:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR_SAT;
			case StencilOp::INCR:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR;
			case StencilOp::INCR_SAT:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR_SAT;
			case StencilOp::INVERT:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_INVERT;
			case StencilOp::REPLACE:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_REPLACE;
			case StencilOp::ZERO:
				return D3D12_STENCIL_OP::D3D12_STENCIL_OP_ZERO;
			}
		}

		static D3D12_COMPARISON_FUNC GetComparisonFunc(ComparisonFunc func)
		{
			switch (func.func)
			{
			default:
			case ComparisonFunc::ALWAYS:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;
			case ComparisonFunc::EQUAL:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_EQUAL;
			case ComparisonFunc::GREATER:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER;
			case ComparisonFunc::GREATER_EQUAL:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case ComparisonFunc::LESS:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
			case ComparisonFunc::LESS_EQUAL:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case ComparisonFunc::NEVER:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
			case ComparisonFunc::NOT_EQUAL:
				return D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NOT_EQUAL;
			}
		}
		static D3D12_DEPTH_WRITE_MASK GetDepthWriteMask(DepthWriteMask mask)
		{
			switch (mask.mask)
			{
			default:
			case DepthWriteMask::ALL:
				return D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;
			case DepthWriteMask::ZERO:
				return D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
			}
		}
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopology(PrimitiveTopology topology)
		{
			switch (topology.primitive)
			{
			default:
			case PrimitiveTopology::POINT:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			case PrimitiveTopology::LINE:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			case PrimitiveTopology::TRIANGLE:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			case PrimitiveTopology::PATCH:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			}
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
		static D3D12_CULL_MODE GetCullMode(CullMode mode)
		{
			switch (mode.mode)
			{
			default:
			case CullMode::BACK:
				return D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
			case CullMode::FRONT:
				return D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
			case CullMode::NONE:
				return D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
			}
		}
		static D3D12_FILL_MODE GetFillMode(FillMode mode)
		{
			switch (mode.mode)
			{
			default:
			case FillMode::SOLID:
				return D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
			case FillMode::WIREFRAME:
				return D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
			}
		}

		static D3D12_CONSERVATIVE_RASTERIZATION_MODE GetConservativeRaster(ConservativeRaster conservative)
		{
			switch (conservative.mode)
			{
			default:
			case ConservativeRaster::OFF:
				return D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			case ConservativeRaster::ON:
				return D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
			}
		}

		static DXGI_FORMAT GetTextureFormat(TextureFormat format)
		{
			static const std::unordered_map<TextureFormat::Format, DXGI_FORMAT> mapping = {
				{ TextureFormat::R32G32B32A32_TYPELESS,					DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_TYPELESS				},
				{ TextureFormat::R32G32B32A32_FLOAT,					DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT					},
				{ TextureFormat::R32G32B32A32_UINT,						DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT					},
				{ TextureFormat::R32G32B32A32_SINT,						DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_SINT					},
				{ TextureFormat::R32G32B32_TYPELESS,					DXGI_FORMAT::DXGI_FORMAT_R32G32B32_TYPELESS					},
				{ TextureFormat::R32G32B32_FLOAT,						DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT					},
				{ TextureFormat::R32G32B32_UINT,						DXGI_FORMAT::DXGI_FORMAT_R32G32B32_UINT						},
				{ TextureFormat::R32G32B32_SINT,						DXGI_FORMAT::DXGI_FORMAT_R32G32B32_SINT						},
				{ TextureFormat::R16G16B16A16_TYPELESS,					DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_TYPELESS				},
				{ TextureFormat::R16G16B16A16_FLOAT,					DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT					},
				{ TextureFormat::R16G16B16A16_UNORM,					DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UNORM					},
				{ TextureFormat::R16G16B16A16_UINT,						DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UINT					},
				{ TextureFormat::R16G16B16A16_SNORM,					DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SNORM					},
				{ TextureFormat::R16G16B16A16_SINT,						DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_SINT					},
				{ TextureFormat::R32G32_TYPELESS,						DXGI_FORMAT::DXGI_FORMAT_R32G32_TYPELESS					},
				{ TextureFormat::R32G32_FLOAT,							DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT						},
				{ TextureFormat::R32G32_UINT,							DXGI_FORMAT::DXGI_FORMAT_R32G32_UINT						},
				{ TextureFormat::R32G32_SINT,							DXGI_FORMAT::DXGI_FORMAT_R32G32_SINT						},
				{ TextureFormat::R32G8X24_TYPELESS,						DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS					},
				{ TextureFormat::D32_FLOAT_S8X24_UINT,					DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT				},
				{ TextureFormat::R32_FLOAT_X8X24_TYPELESS,				DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS			},
				{ TextureFormat::X32_TYPELESS_G8X24_UINT,				DXGI_FORMAT::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT			},
				{ TextureFormat::R10G10B10A2_TYPELESS,					DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_TYPELESS				},
				{ TextureFormat::R10G10B10A2_UNORM,						DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UNORM					},
				{ TextureFormat::R10G10B10A2_UINT,						DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UINT					},
				{ TextureFormat::R11G11B10_FLOAT,						DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT					},
				{ TextureFormat::R8G8B8A8_TYPELESS,						DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS					},
				{ TextureFormat::R8G8B8A8_UNORM,						DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM						},
				{ TextureFormat::R8G8B8A8_UNORM_SRGB,					DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB				},
				{ TextureFormat::R8G8B8A8_UINT,							DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT						},
				{ TextureFormat::R8G8B8A8_SNORM,						DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SNORM						},
				{ TextureFormat::R8G8B8A8_SINT,							DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SINT						},
				{ TextureFormat::R16G16_TYPELESS,						DXGI_FORMAT::DXGI_FORMAT_R16G16_TYPELESS					},
				{ TextureFormat::R16G16_FLOAT,							DXGI_FORMAT::DXGI_FORMAT_R16G16_FLOAT						},
				{ TextureFormat::R16G16_UNORM,							DXGI_FORMAT::DXGI_FORMAT_R16G16_UNORM						},
				{ TextureFormat::R16G16_UINT,							DXGI_FORMAT::DXGI_FORMAT_R16G16_UINT						},
				{ TextureFormat::R16G16_SNORM,							DXGI_FORMAT::DXGI_FORMAT_R16G16_SNORM						},
				{ TextureFormat::R16G16_SINT,							DXGI_FORMAT::DXGI_FORMAT_R16G16_SINT						},
				{ TextureFormat::R32_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS						},
				{ TextureFormat::D32_FLOAT,								DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT							},
				{ TextureFormat::R32_FLOAT,								DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT							},
				{ TextureFormat::R32_UINT,								DXGI_FORMAT::DXGI_FORMAT_R32_UINT							},
				{ TextureFormat::R32_SINT,								DXGI_FORMAT::DXGI_FORMAT_R32_SINT							},
				{ TextureFormat::R24G8_TYPELESS,						DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS						},
				{ TextureFormat::D24_UNORM_S8_UINT,						DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT					},
				{ TextureFormat::R24_UNORM_X8_TYPELESS,					DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS				},
				{ TextureFormat::X24_TYPELESS_G8_UINT,					DXGI_FORMAT::DXGI_FORMAT_X24_TYPELESS_G8_UINT				},
				{ TextureFormat::R8G8_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_R8G8_TYPELESS						},
				{ TextureFormat::R8G8_UNORM,							DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM							},
				{ TextureFormat::R8G8_UINT,								DXGI_FORMAT::DXGI_FORMAT_R8G8_UINT							},
				{ TextureFormat::R8G8_SNORM,							DXGI_FORMAT::DXGI_FORMAT_R8G8_SNORM							},
				{ TextureFormat::R8G8_SINT,								DXGI_FORMAT::DXGI_FORMAT_R8G8_SINT							},
				{ TextureFormat::R16_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS						},
				{ TextureFormat::R16_FLOAT,								DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT							},
				{ TextureFormat::D16_UNORM,								DXGI_FORMAT::DXGI_FORMAT_D16_UNORM							},
				{ TextureFormat::R16_UNORM,								DXGI_FORMAT::DXGI_FORMAT_R16_UNORM							},
				{ TextureFormat::R16_UINT,								DXGI_FORMAT::DXGI_FORMAT_R16_UINT							},
				{ TextureFormat::R16_SNORM,								DXGI_FORMAT::DXGI_FORMAT_R16_SNORM							},
				{ TextureFormat::R16_SINT,								DXGI_FORMAT::DXGI_FORMAT_R16_SINT							},
				{ TextureFormat::R8_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_R8_TYPELESS						},
				{ TextureFormat::R8_UNORM,								DXGI_FORMAT::DXGI_FORMAT_R8_UNORM							},
				{ TextureFormat::R8_UINT,								DXGI_FORMAT::DXGI_FORMAT_R8_UINT							},
				{ TextureFormat::R8_SNORM,								DXGI_FORMAT::DXGI_FORMAT_R8_SNORM							},
				{ TextureFormat::R8_SINT,								DXGI_FORMAT::DXGI_FORMAT_R8_SINT							},
				{ TextureFormat::A8_UNORM,								DXGI_FORMAT::DXGI_FORMAT_A8_UNORM							},
				{ TextureFormat::R1_UNORM,								DXGI_FORMAT::DXGI_FORMAT_R1_UNORM							},
				{ TextureFormat::R9G9B9E5_SHAREDEXP,					DXGI_FORMAT::DXGI_FORMAT_R9G9B9E5_SHAREDEXP					},
				{ TextureFormat::R8G8_B8G8_UNORM,						DXGI_FORMAT::DXGI_FORMAT_R8G8_B8G8_UNORM					},
				{ TextureFormat::G8R8_G8B8_UNORM,						DXGI_FORMAT::DXGI_FORMAT_G8R8_G8B8_UNORM					},
				{ TextureFormat::BC1_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_BC1_TYPELESS						},
				{ TextureFormat::BC1_UNORM,								DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM							},
				{ TextureFormat::BC1_UNORM_SRGB,						DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM_SRGB						},
				{ TextureFormat::BC2_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_BC2_TYPELESS						},
				{ TextureFormat::BC2_UNORM,								DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM							},
				{ TextureFormat::BC2_UNORM_SRGB,						DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM_SRGB						},
				{ TextureFormat::BC3_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_BC3_TYPELESS						},
				{ TextureFormat::BC3_UNORM,								DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM							},
				{ TextureFormat::BC3_UNORM_SRGB,						DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM_SRGB						},
				{ TextureFormat::BC4_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_BC4_TYPELESS						},
				{ TextureFormat::BC4_UNORM,								DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM							},
				{ TextureFormat::BC4_SNORM,								DXGI_FORMAT::DXGI_FORMAT_BC4_SNORM							},
				{ TextureFormat::BC5_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_BC5_TYPELESS						},
				{ TextureFormat::BC5_UNORM,								DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM							},
				{ TextureFormat::BC5_SNORM,								DXGI_FORMAT::DXGI_FORMAT_BC5_SNORM							},
				{ TextureFormat::B5G6R5_UNORM,							DXGI_FORMAT::DXGI_FORMAT_B5G6R5_UNORM						},
				{ TextureFormat::B5G5R5A1_UNORM,						DXGI_FORMAT::DXGI_FORMAT_B5G5R5A1_UNORM						},
				{ TextureFormat::B8G8R8A8_UNORM,						DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM						},
				{ TextureFormat::B8G8R8X8_UNORM,						DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM						},
				{ TextureFormat::R10G10B10_XR_BIAS_A2_UNORM,			DXGI_FORMAT::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM			},
				{ TextureFormat::B8G8R8A8_TYPELESS,						DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_TYPELESS					},
				{ TextureFormat::B8G8R8A8_UNORM_SRGB,					DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB				},
				{ TextureFormat::B8G8R8X8_TYPELESS,						DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_TYPELESS					},
				{ TextureFormat::B8G8R8X8_UNORM_SRGB,					DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB				},
				{ TextureFormat::BC6H_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_BC6H_TYPELESS						},
				{ TextureFormat::BC6H_UF16,								DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16							},
				{ TextureFormat::BC6H_SF16,								DXGI_FORMAT::DXGI_FORMAT_BC6H_SF16							},
				{ TextureFormat::BC7_TYPELESS,							DXGI_FORMAT::DXGI_FORMAT_BC7_TYPELESS						},
				{ TextureFormat::BC7_UNORM,								DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM							},
				{ TextureFormat::BC7_UNORM_SRGB,						DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB						},
				{ TextureFormat::AYUV,									DXGI_FORMAT::DXGI_FORMAT_AYUV								},
				{ TextureFormat::Y410,									DXGI_FORMAT::DXGI_FORMAT_Y410								},
				{ TextureFormat::Y416,									DXGI_FORMAT::DXGI_FORMAT_Y416								},
				{ TextureFormat::NV12,									DXGI_FORMAT::DXGI_FORMAT_NV12								},
				{ TextureFormat::P010,									DXGI_FORMAT::DXGI_FORMAT_P010								},
				{ TextureFormat::P016,									DXGI_FORMAT::DXGI_FORMAT_P016								},
				{ TextureFormat::_420_OPAQUE,							DXGI_FORMAT::DXGI_FORMAT_420_OPAQUE							},
				{ TextureFormat::YUY2,									DXGI_FORMAT::DXGI_FORMAT_YUY2								},
				{ TextureFormat::Y210,									DXGI_FORMAT::DXGI_FORMAT_Y210								},
				{ TextureFormat::Y216,									DXGI_FORMAT::DXGI_FORMAT_Y216								},
				{ TextureFormat::NV11,									DXGI_FORMAT::DXGI_FORMAT_NV11								},
				{ TextureFormat::AI44,									DXGI_FORMAT::DXGI_FORMAT_AI44								},
				{ TextureFormat::IA44,									DXGI_FORMAT::DXGI_FORMAT_IA44								},
				{ TextureFormat::P8,									DXGI_FORMAT::DXGI_FORMAT_P8									},
				{ TextureFormat::A8P8,									DXGI_FORMAT::DXGI_FORMAT_A8P8								},
				{ TextureFormat::B4G4R4A4_UNORM,						DXGI_FORMAT::DXGI_FORMAT_B4G4R4A4_UNORM						},
				{ TextureFormat::P208,									DXGI_FORMAT::DXGI_FORMAT_P208								},
				{ TextureFormat::V208,									DXGI_FORMAT::DXGI_FORMAT_V208								},
				{ TextureFormat::V408,									DXGI_FORMAT::DXGI_FORMAT_V408								},
				{ TextureFormat::INVALID,								DXGI_FORMAT::DXGI_FORMAT_UNKNOWN							}
			};

			const auto& it = mapping.find(format.format);
			if (it == mapping.end())
			{
				Themp::Print("Invalid format given!");
				return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
			}
			return it->second;

		}
	};
}
