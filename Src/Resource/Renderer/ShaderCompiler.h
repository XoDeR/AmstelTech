#pragma once

#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Core/Containers/Map.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/StringStream.h"

#include "Resource/Types.h"

#include "RioRenderer/RioRenderer.h"

#include "Config.h"

#include "Device/Device.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

#include "Resource/ShaderResource.h"

#include "World/ShaderManager.h"

#if RIO_DEVELOPMENT
#define SHADER_COMPILER_NAME "ShaderCompilerDevelopment"
#elif RIO_DEBUG
#define SHADER_COMPILER_NAME "ShaderCompilerDebug"
#else
#define SHADER_COMPILER_NAME "ShaderCompilerRelease"
#endif  // RIO_DEBUG

#if RIO_PLATFORM_LINUX
#define SHADER_COMPILER_PATH "./" SHADER_COMPILER_NAME ""
#elif RIO_PLATFORM_WINDOWS
#define SHADER_COMPILER_PATH SHADER_COMPILER_NAME ".exe"
#else
#define SHADER_COMPILER_PATH ""
#endif // RIO_PLATFORM_LINUX

namespace Rio
{

namespace ShaderResourceInternalFn
{

	struct DepthFunction
	{
		enum Enum
		{
			LESS,
			LEQUAL,
			EQUAL,
			GEQUAL,
			GREATER,
			NOTEQUAL,
			NEVER,
			ALWAYS,

			COUNT
		};
	};

	struct BlendFunction
	{
		enum Enum
		{
			ZERO,
			ONE,
			SRC_COLOR,
			INV_SRC_COLOR,
			SRC_ALPHA,
			INV_SRC_ALPHA,
			DST_ALPHA,
			INV_DST_ALPHA,
			DST_COLOR,
			INV_DST_COLOR,
			SRC_ALPHA_SAT,
			FACTOR,
			INV_FACTOR,

			COUNT
		};
	};

	struct BlendEquation
	{
		enum Enum
		{
			ADD,
			SUB,
			REVSUB,
			MIN,
			MAX,

			COUNT
		};
	};

	struct CullMode
	{
		enum Enum
		{
			CW,
			CCW,
			NONE,

			COUNT
		};
	};

	struct PrimitiveType
	{
		enum Enum
		{
			PT_TRISTRIP,
			PT_LINES,
			PT_LINESTRIP,
			PT_POINTS,

			COUNT
		};
	};

	struct SamplerFilter
	{
		enum Enum
		{
			POINT,
			ANISOTROPIC,

			COUNT
		};
	};

	struct SamplerWrap
	{
		enum Enum
		{
			MIRROR,
			CLAMP,
			BORDER,

			COUNT
		};
	};

	struct DepthTestInfo
	{
		const char* name = nullptr;
		DepthFunction::Enum value = DepthFunction::COUNT;
	};

	static const DepthTestInfo depthTestMap[] =
	{
		{ "less", DepthFunction::LESS },
		{ "lessEqual", DepthFunction::LEQUAL },
		{ "equal", DepthFunction::EQUAL },
		{ "greaterEqual", DepthFunction::GEQUAL },
		{ "greater", DepthFunction::GREATER },
		{ "notEqual", DepthFunction::NOTEQUAL },
		{ "never", DepthFunction::NEVER },
		{ "always", DepthFunction::ALWAYS }
	};
	RIO_STATIC_ASSERT(countof(depthTestMap) == DepthFunction::COUNT);

	struct BlendFunctionInfo
	{
		const char* name = nullptr;
		BlendFunction::Enum value = BlendFunction::COUNT;
	};

	static const BlendFunctionInfo blendFunctionMap[] =
	{
		{ "zero", BlendFunction::ZERO },
		{ "one", BlendFunction::ONE },
		{ "srcColor", BlendFunction::SRC_COLOR },
		{ "invSrcColor", BlendFunction::INV_SRC_COLOR },
		{ "srcAlpha", BlendFunction::SRC_ALPHA },
		{ "invSrcAlpha", BlendFunction::INV_SRC_ALPHA },
		{ "dstAlpha", BlendFunction::DST_ALPHA },
		{ "invDstAlpha", BlendFunction::INV_DST_ALPHA },
		{ "dstColor", BlendFunction::DST_COLOR },
		{ "invDstColor", BlendFunction::INV_DST_COLOR },
		{ "srcAlphaSat", BlendFunction::SRC_ALPHA_SAT },
		{ "factor", BlendFunction::FACTOR },
		{ "invFactor", BlendFunction::INV_FACTOR }
	};
	RIO_STATIC_ASSERT(countof(blendFunctionMap) == BlendFunction::COUNT);

	struct BlendEquationInfo
	{
		const char* name = nullptr;
		BlendEquation::Enum value = BlendEquation::COUNT;
	};

	static const BlendEquationInfo blendEquationMap[] =
	{
		{ "add", BlendEquation::ADD },
		{ "sub", BlendEquation::SUB },
		{ "revSub", BlendEquation::REVSUB },
		{ "min", BlendEquation::MIN },
		{ "max", BlendEquation::MAX }
	};
	RIO_STATIC_ASSERT(countof(blendEquationMap) == BlendEquation::COUNT);

	struct CullModeInfo
	{
		const char* name = nullptr;
		CullMode::Enum value = CullMode::COUNT;
	};

	static const CullModeInfo cullModeInfoMap[] =
	{
		{ "cw", CullMode::CW },
		{ "ccw", CullMode::CCW },
		{ "none", CullMode::NONE }
	};
	RIO_STATIC_ASSERT(countof(cullModeInfoMap) == CullMode::COUNT);

	struct PrimitiveTypeInfo
	{
		const char* name = nullptr;
		PrimitiveType::Enum value = PrimitiveType::COUNT;
	};

	static const PrimitiveTypeInfo primitiveTypeInfoMap[] =
	{
		{ "ptTriStrip", PrimitiveType::PT_TRISTRIP },
		{ "ptLines", PrimitiveType::PT_LINES },
		{ "ptLineStrip", PrimitiveType::PT_LINESTRIP },
		{ "ptPoints", PrimitiveType::PT_POINTS }
	};
	RIO_STATIC_ASSERT(countof(primitiveTypeInfoMap) == PrimitiveType::COUNT);

	struct SamplerFilterInfo
	{
		const char* name = nullptr;
		SamplerFilter::Enum value = SamplerFilter::COUNT;
	};

	static const SamplerFilterInfo samplerFilterInfoMap[] =
	{
		{ "point", SamplerFilter::POINT },
		{ "anisotropic", SamplerFilter::ANISOTROPIC }
	};
	RIO_STATIC_ASSERT(countof(samplerFilterInfoMap) == SamplerFilter::COUNT);

	struct SamplerWrapInfo
	{
		const char* name = nullptr;
		SamplerWrap::Enum value = SamplerWrap::COUNT;
	};

	static const SamplerWrapInfo samplerWrapInfoMap[] =
	{
		{ "mirror", SamplerWrap::MIRROR },
		{ "clamp", SamplerWrap::CLAMP },
		{ "border", SamplerWrap::BORDER }
	};
	RIO_STATIC_ASSERT(countof(samplerWrapInfoMap) == SamplerWrap::COUNT);

	static const uint64_t rioRendererDepthFunctionMap[] =
	{
		RIO_RENDERER_STATE_DEPTH_TEST_LESS, // DepthFunction::LESS
		RIO_RENDERER_STATE_DEPTH_TEST_LEQUAL, // DepthFunction::LEQUAL
		RIO_RENDERER_STATE_DEPTH_TEST_EQUAL, // DepthFunction::EQUAL
		RIO_RENDERER_STATE_DEPTH_TEST_GEQUAL, // DepthFunction::GEQUAL
		RIO_RENDERER_STATE_DEPTH_TEST_GREATER, // DepthFunction::GREATER
		RIO_RENDERER_STATE_DEPTH_TEST_NOTEQUAL, // DepthFunction::NOTEQUAL
		RIO_RENDERER_STATE_DEPTH_TEST_NEVER, // DepthFunction::NEVER
		RIO_RENDERER_STATE_DEPTH_TEST_ALWAYS // DepthFunction::ALWAYS
	};
	RIO_STATIC_ASSERT(countof(rioRendererDepthFunctionMap) == DepthFunction::COUNT);

	static const uint64_t rioRendererBlendFunctionMap[] =
	{
		RIO_RENDERER_STATE_BLEND_ZERO, // BlendFunction::ZERO
		RIO_RENDERER_STATE_BLEND_ONE, // BlendFunction::ONE
		RIO_RENDERER_STATE_BLEND_SRC_COLOR, // BlendFunction::SRC_COLOR
		RIO_RENDERER_STATE_BLEND_INV_SRC_COLOR, // BlendFunction::INV_SRC_COLOR
		RIO_RENDERER_STATE_BLEND_SRC_ALPHA, // BlendFunction::SRC_ALPHA
		RIO_RENDERER_STATE_BLEND_INV_SRC_ALPHA, // BlendFunction::INV_SRC_ALPHA
		RIO_RENDERER_STATE_BLEND_DST_ALPHA, // BlendFunction::DST_ALPHA
		RIO_RENDERER_STATE_BLEND_INV_DST_ALPHA, // BlendFunction::INV_DST_ALPHA
		RIO_RENDERER_STATE_BLEND_DST_COLOR, // BlendFunction::DST_COLOR
		RIO_RENDERER_STATE_BLEND_INV_DST_COLOR, // BlendFunction::INV_DST_COLOR
		RIO_RENDERER_STATE_BLEND_SRC_ALPHA_SAT, // BlendFunction::SRC_ALPHA_SAT
		RIO_RENDERER_STATE_BLEND_FACTOR, // BlendFunction::FACTOR
		RIO_RENDERER_STATE_BLEND_INV_FACTOR // BlendFunction::INV_FACTOR
	};
	RIO_STATIC_ASSERT(countof(rioRendererBlendFunctionMap) == BlendFunction::COUNT);

	static const uint64_t rioRendererBlendEquationMap[] =
	{
		RIO_RENDERER_STATE_BLEND_EQUATION_ADD,    // BlendEquation::ADD
		RIO_RENDERER_STATE_BLEND_EQUATION_SUB,    // BlendEquation::SUB
		RIO_RENDERER_STATE_BLEND_EQUATION_REVSUB, // BlendEquation::REVSUB
		RIO_RENDERER_STATE_BLEND_EQUATION_MIN,    // BlendEquation::MIN
		RIO_RENDERER_STATE_BLEND_EQUATION_MAX     // BlendEquation::MAX
	};
	RIO_STATIC_ASSERT(countof(rioRendererBlendEquationMap) == BlendEquation::COUNT);

	static const uint64_t rioRendererCullModeMap[] =
	{
		RIO_RENDERER_STATE_CULL_CW, // CullMode::CW
		RIO_RENDERER_STATE_CULL_CCW, // CullMode::CCW
		0 // CullMode::NONE
	};
	RIO_STATIC_ASSERT(countof(rioRendererCullModeMap) == CullMode::COUNT);

	static const uint64_t rioRendererPrimitiveTypeMap[] =
	{
		RIO_RENDERER_STATE_PT_TRISTRIP, // PrimitiveType::PT_TRISTRIP
		RIO_RENDERER_STATE_PT_LINES, // PrimitiveType::PT_LINES
		RIO_RENDERER_STATE_PT_LINESTRIP, // PrimitiveType::PT_LINESTRIP
		RIO_RENDERER_STATE_PT_POINTS // PrimitiveType::PT_POINTS
	};
	RIO_STATIC_ASSERT(countof(rioRendererPrimitiveTypeMap) == PrimitiveType::COUNT);

	static const uint32_t rioRendererSamplerFilterMinMap[] =
	{
		RIO_RENDERER_TEXTURE_MIN_POINT, // SamplerFilter::POINT
		RIO_RENDERER_TEXTURE_MIN_ANISOTROPIC // SamplerFilter::ANISOTROPIC
	};
	RIO_STATIC_ASSERT(countof(rioRendererSamplerFilterMinMap) == SamplerFilter::COUNT);

	static const uint32_t rioRendererSamplerFilterMagMap[] =
	{
		RIO_RENDERER_TEXTURE_MAG_POINT, // SamplerFilter::POINT
		RIO_RENDERER_TEXTURE_MAG_ANISOTROPIC // SamplerFilter::ANISOTROPIC
	};
	RIO_STATIC_ASSERT(countof(rioRendererSamplerFilterMagMap) == SamplerFilter::COUNT);

	static const uint32_t rioRendererSamplerWrapUMap[] =
	{
		RIO_RENDERER_TEXTURE_U_MIRROR, // SamplerWrap::MIRROR
		RIO_RENDERER_TEXTURE_U_CLAMP, // SamplerWrap::CLAMP
		RIO_RENDERER_TEXTURE_U_BORDER // SamplerWrap::BORDER
	};
	RIO_STATIC_ASSERT(countof(rioRendererSamplerWrapUMap) == SamplerWrap::COUNT);

	static const uint32_t rioRendererSamplerWrapVMap[] =
	{
		RIO_RENDERER_TEXTURE_V_MIRROR, // SamplerWrap::MIRROR
		RIO_RENDERER_TEXTURE_V_CLAMP, // SamplerWrap::CLAMP
		RIO_RENDERER_TEXTURE_V_BORDER  // SamplerWrap::BORDER
	};
	RIO_STATIC_ASSERT(countof(rioRendererSamplerWrapVMap) == SamplerWrap::COUNT);

	static const uint32_t rioRendererSamplerWrapWMap[] =
	{
		RIO_RENDERER_TEXTURE_W_MIRROR, // SamplerWrap::MIRROR
		RIO_RENDERER_TEXTURE_W_CLAMP, // SamplerWrap::CLAMP
		RIO_RENDERER_TEXTURE_W_BORDER  // SamplerWrap::BORDER
	};
	RIO_STATIC_ASSERT(countof(rioRendererSamplerWrapWMap) == SamplerWrap::COUNT);

	static DepthFunction::Enum getDepthFunctionByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(depthTestMap); ++i)
		{
			if (strcmp(name, depthTestMap[i].name) == 0)
			{
				return depthTestMap[i].value;
			}
		}

		return DepthFunction::COUNT;
	}

	static BlendFunction::Enum getBlendFunctionByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(blendFunctionMap); ++i)
		{
			if (strcmp(name, blendFunctionMap[i].name) == 0)
			{
				return blendFunctionMap[i].value;
			}
		}

		return BlendFunction::COUNT;
	}

	static BlendEquation::Enum getBlendEquationByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(blendEquationMap); ++i)
		{
			if (strcmp(name, blendEquationMap[i].name) == 0)
			{
				return blendEquationMap[i].value;
			}
		}

		return BlendEquation::COUNT;
	}

	static CullMode::Enum getCullModeByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(cullModeInfoMap); ++i)
		{
			if (strcmp(name, cullModeInfoMap[i].name) == 0)
			{
				return cullModeInfoMap[i].value;
			}
		}

		return CullMode::COUNT;
	}

	static PrimitiveType::Enum getPrimitiveTypeByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(primitiveTypeInfoMap); ++i)
		{
			if (strcmp(name, primitiveTypeInfoMap[i].name) == 0)
			{
				return primitiveTypeInfoMap[i].value;
			}
		}

		return PrimitiveType::COUNT;
	}

	static SamplerFilter::Enum getSamplerFilterByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(samplerFilterInfoMap); ++i)
		{
			if (strcmp(name, samplerFilterInfoMap[i].name) == 0)
			{
				return samplerFilterInfoMap[i].value;
			}
		}

		return SamplerFilter::COUNT;
	}

	static SamplerWrap::Enum getSamplerWrapByName(const char* name)
	{
		for (uint32_t i = 0; i < countof(samplerWrapInfoMap); ++i)
		{
			if (strcmp(name, samplerWrapInfoMap[i].name) == 0)
			{
				return samplerWrapInfoMap[i].value;
			}
		}

		return SamplerWrap::COUNT;
	}

	static int runExternalCompiler(CompileOptions& compileOptions, const char* infile, const char* outfile, const char* varying, const char* type, const char* platformName, StringStream& outputStringStream)
	{
		const char* argumentList[] =
		{
			SHADER_COMPILER_PATH,
			"-f",
			infile,
			"-o",
			outfile,
			"--varyingdef",
			varying,
			"--type",
			type,
			"--platform",
			platformName,
			nullptr,
			nullptr,
			nullptr,
		};

		if (strcmp("windows", platformName) == 0)
		{
			argumentList[11] = "--profile";
			argumentList[12] = ((strcmp(type, "vertex") == 0) ? "vs_4_0" : "ps_4_0");
		}

		return compileOptions.runExternalCompiler(argumentList, outputStringStream);
	}

	struct RenderState
	{
		bool rgbWriteEnable = false;
		bool alphaWriteEnable = false;
		bool depthWriteEnable = false;
		bool depthEnable = false;
		bool blendEnable = false;
		DepthFunction::Enum depthFunction = DepthFunction::COUNT;
		BlendFunction::Enum blendSource = BlendFunction::COUNT;
		BlendFunction::Enum blendDestination = BlendFunction::COUNT;
		BlendEquation::Enum blendEquation = BlendEquation::COUNT;
		CullMode::Enum cullMode = CullMode::COUNT;
		PrimitiveType::Enum primitiveType = PrimitiveType::COUNT;

		void reset()
		{
			this->rgbWriteEnable = false;
			this->alphaWriteEnable = false;
			this->depthWriteEnable = false;
			this->depthEnable = false;
			this->blendEnable = false;
			this->depthFunction = DepthFunction::COUNT;
			this->blendSource = BlendFunction::COUNT;
			this->blendDestination = BlendFunction::COUNT;
			this->blendEquation = BlendEquation::COUNT;
			this->cullMode = CullMode::COUNT;
			this->primitiveType = PrimitiveType::COUNT;
		}

		uint64_t encode() const
		{
			const uint64_t depthFunctionIndex = (this->depthEnable
				? rioRendererDepthFunctionMap[depthFunction]
				: 0
				);
			const uint64_t blendFunction = (this->blendEnable
				? RIO_RENDERER_STATE_BLEND_FUNC(rioRendererBlendFunctionMap[this->blendSource], rioRendererBlendFunctionMap[this->blendDestination])
				: 0
				);
			const uint64_t blendEquation = (this->blendEnable
				? RIO_RENDERER_STATE_BLEND_EQUATION(rioRendererBlendEquationMap[this->blendEquation])
				: 0
				);
			const uint64_t cullMode = (this->cullMode != CullMode::COUNT
				? rioRendererCullModeMap[this->cullMode]
				: 0
				);
			const uint64_t primitiveTypeIndex = (this->primitiveType != PrimitiveType::COUNT
				? rioRendererPrimitiveTypeMap[this->primitiveType]
				: 0
				);

			uint64_t state = 0;
			state |= (this->rgbWriteEnable ? RIO_RENDERER_STATE_RGB_WRITE : 0);
			state |= (this->alphaWriteEnable ? RIO_RENDERER_STATE_ALPHA_WRITE : 0);
			state |= (this->depthWriteEnable ? RIO_RENDERER_STATE_DEPTH_WRITE : 0);
			state |= depthFunctionIndex;
			state |= blendFunction;
			state |= blendEquation;
			state |= cullMode;
			state |= primitiveTypeIndex;

			return state;
		}
	};

	struct SamplerState
	{
		SamplerFilter::Enum filterMin = SamplerFilter::COUNT;
		SamplerFilter::Enum filterMag = SamplerFilter::COUNT;
		SamplerWrap::Enum wrapU = SamplerWrap::COUNT;
		SamplerWrap::Enum wrapV = SamplerWrap::COUNT;
		SamplerWrap::Enum wrapW = SamplerWrap::COUNT;

		void reset()
		{
			this->filterMin = SamplerFilter::COUNT;
			this->filterMag = SamplerFilter::COUNT;
			this->wrapU = SamplerWrap::COUNT;
			this->wrapV = SamplerWrap::COUNT;
			this->wrapW = SamplerWrap::COUNT;
		}

		uint32_t encode() const
		{
			uint32_t state = 0;
			state |= this->filterMin == SamplerFilter::COUNT ? 0 : rioRendererSamplerFilterMinMap[this->filterMin];
			state |= this->filterMag == SamplerFilter::COUNT ? 0 : rioRendererSamplerFilterMagMap[this->filterMag];
			state |= this->wrapU == SamplerWrap::COUNT ? 0 : rioRendererSamplerWrapUMap[ this->wrapU];
			state |= this->wrapV == SamplerWrap::COUNT ? 0 : rioRendererSamplerWrapVMap[ this->wrapV];
			state |= this->wrapW == SamplerWrap::COUNT ? 0 : rioRendererSamplerWrapWMap[ this->wrapW];
			return state;
		}
	};

	struct RioRendererShader
	{
		ALLOCATOR_AWARE;

		DynamicString includeListString;
		DynamicString codeString;
		DynamicString vertexShaderCode;
		DynamicString fragmentShaderCodeString;
		DynamicString varyingString;
		DynamicString vertexShaderInputOutput;
		DynamicString fragmentShaderInputOutput;
		Map<DynamicString, DynamicString> samplerMap;

		RioRendererShader()
			: includeListString(getDefaultAllocator())
			, codeString(getDefaultAllocator())
			, vertexShaderCode(getDefaultAllocator())
			, fragmentShaderCodeString(getDefaultAllocator())
			, varyingString(getDefaultAllocator())
			, vertexShaderInputOutput(getDefaultAllocator())
			, fragmentShaderInputOutput(getDefaultAllocator())
			, samplerMap(getDefaultAllocator())
		{
		}

		RioRendererShader(Allocator& a)
			: includeListString(a)
			, codeString(a)
			, vertexShaderCode(a)
			, fragmentShaderCodeString(a)
			, varyingString(a)
			, vertexShaderInputOutput(a)
			, fragmentShaderInputOutput(a)
			, samplerMap(a)
		{
		}
	};

	struct ShaderPermutation
	{
		ALLOCATOR_AWARE;

		DynamicString rioRendererShaderName;
		DynamicString renderStateString;

		ShaderPermutation()
			: rioRendererShaderName(getDefaultAllocator())
			, renderStateString(getDefaultAllocator())
		{
		}

		ShaderPermutation(Allocator& a)
			: rioRendererShaderName(a)
			, renderStateString(a)
		{
		}
	};

	struct StaticCompile
	{
		DynamicString shaderString;
		Vector<DynamicString> defineList;

		StaticCompile()
			: shaderString(getDefaultAllocator())
			, defineList(getDefaultAllocator())
		{
		}

		StaticCompile(Allocator& a)
			: shaderString(a)
			, defineList(a)
		{
		}
	};

	struct ShaderCompiler
	{
		CompileOptions& compileOptions;
		Map<DynamicString, RenderState> renderStateMap;
		Map<DynamicString, SamplerState> samplerStateMap;
		Map<DynamicString, RioRendererShader> rioRendererShaderMap;
		Map<DynamicString, ShaderPermutation> shaderPermutationMap;
		Vector<StaticCompile> staticCompileList;

		DynamicString vertexShaderSourcePath;
		DynamicString fragmentShaderSourcePath;
		DynamicString varyingPath;
		DynamicString vertexShaderCompiledPath;
		DynamicString fragmentShaderCompiledPath;

		ShaderCompiler(CompileOptions& compileOptions)
			: compileOptions(compileOptions)
			, renderStateMap(getDefaultAllocator())
			, samplerStateMap(getDefaultAllocator())
			, rioRendererShaderMap(getDefaultAllocator())
			, shaderPermutationMap(getDefaultAllocator())
			, staticCompileList(getDefaultAllocator())
			, vertexShaderSourcePath(getDefaultAllocator())
			, fragmentShaderSourcePath(getDefaultAllocator())
			, varyingPath(getDefaultAllocator())
			, vertexShaderCompiledPath(getDefaultAllocator())
			, fragmentShaderCompiledPath(getDefaultAllocator())
		{
			compileOptions.getTemporaryPath("vsSource.sc", this->vertexShaderSourcePath);
			compileOptions.getTemporaryPath("fsSource.sc", this->fragmentShaderSourcePath);
			compileOptions.getTemporaryPath("varying.sc", this->varyingPath);
			compileOptions.getTemporaryPath("vsCompiled.bin", this->vertexShaderCompiledPath);
			compileOptions.getTemporaryPath("fsCompiled.bin", this->fragmentShaderCompiledPath);
		}

		void parse(const char* path)
		{
			parse(compileOptions.read(path));
		}

		void parse(Buffer buffer)
		{
			TempAllocator4096 tempAllocator4096;
			JsonObject shaderJsonObject(tempAllocator4096);
			RJsonFn::parse(buffer, shaderJsonObject);

			if (JsonObjectFn::has(shaderJsonObject, "include"))
			{
				JsonArray includeJsonArray(tempAllocator4096);
				RJsonFn::parseArray(shaderJsonObject["include"], includeJsonArray);

				for (uint32_t i = 0; i < ArrayFn::getCount(includeJsonArray); ++i)
				{
					DynamicString path(tempAllocator4096);
					RJsonFn::parseString(includeJsonArray[i], path);
					parse(path.getCStr());
				}
			}

			if (JsonObjectFn::has(shaderJsonObject, "renderStateList"))
			{
				parseRenderStateList(shaderJsonObject["renderStateList"]);
			}

			if (JsonObjectFn::has(shaderJsonObject, "samplerStateList"))
			{
				parseSamplerStateList(shaderJsonObject["samplerStateList"]);
			}

			if (JsonObjectFn::has(shaderJsonObject, "rioRendererShaderList"))
			{
				parseRioRendererShaders(shaderJsonObject["rioRendererShaderList"]);
			}

			if (JsonObjectFn::has(shaderJsonObject, "shaderList"))
			{
				parseShaderList(shaderJsonObject["shaderList"]);
			}

			if (JsonObjectFn::has(shaderJsonObject, "staticCompile"))
			{
				parseStaticCompile(shaderJsonObject["staticCompile"]);
			}
		}

	private:

		void parseRenderStateList(const char* json)
		{
			TempAllocator4096 tempAllocator4096;
			JsonObject renderStateListJsonObject(tempAllocator4096);
			RJsonFn::parseObject(json, renderStateListJsonObject);

			auto currentRenderStateJson = JsonObjectFn::begin(renderStateListJsonObject);
			auto endRenderStateJson = JsonObjectFn::end(renderStateListJsonObject);
			for (; currentRenderStateJson != endRenderStateJson; ++currentRenderStateJson)
			{
				JsonObject renderStateJsonObject(tempAllocator4096);
				RJsonFn::parseObject(currentRenderStateJson->pair.second, renderStateJsonObject);

				const bool rgbWriteEnable = RJsonFn::parseBool(renderStateJsonObject["rgbWriteEnable"]);
				const bool alphaWriteEnable = RJsonFn::parseBool(renderStateJsonObject["alphaWriteEnable"]);
				const bool depthWriteEnable = RJsonFn::parseBool(renderStateJsonObject["depthWriteEnable"]);
				const bool depthEnable = RJsonFn::parseBool(renderStateJsonObject["depthEnable"]);
				const bool blendEnable = RJsonFn::parseBool(renderStateJsonObject["blendEnable"]);

				const bool hasDepthFunction = JsonObjectFn::has(renderStateJsonObject, "depthFunction");
				const bool hasBlendSource = JsonObjectFn::has(renderStateJsonObject, "blendSource");
				const bool hasBlendDestination = JsonObjectFn::has(renderStateJsonObject, "blendDestination");
				const bool hasBlendEquation = JsonObjectFn::has(renderStateJsonObject, "blendEquation");
				const bool hasCullMode = JsonObjectFn::has(renderStateJsonObject, "cullMode");
				const bool hasPrimitiveType = JsonObjectFn::has(renderStateJsonObject, "primitiveType");

				RenderState renderState;
				renderState.reset();
				renderState.rgbWriteEnable = rgbWriteEnable;
				renderState.alphaWriteEnable = alphaWriteEnable;
				renderState.depthWriteEnable = depthWriteEnable;
				renderState.depthEnable = depthEnable;
				renderState.blendEnable = blendEnable;

				DynamicString depthFunctionString(tempAllocator4096);
				DynamicString blendSourceString(tempAllocator4096);
				DynamicString blendDestinationString(tempAllocator4096);
				DynamicString blendEquationString(tempAllocator4096);
				DynamicString cullModeString(tempAllocator4096);
				DynamicString primitiveTypeString(tempAllocator4096);

				if (hasDepthFunction == true)
				{
					RJsonFn::parseString(renderStateJsonObject["depthFunction"], depthFunctionString);
					renderState.depthFunction = getDepthFunctionByName(depthFunctionString.getCStr());
					DATA_COMPILER_ASSERT(renderState.depthFunction != DepthFunction::COUNT
						, compileOptions
						, "Unknown depth test: '%s'"
						, depthFunctionString.getCStr()
					);
				}

				if (hasBlendSource == true)
				{
					RJsonFn::parseString(renderStateJsonObject["blendSource"], blendSourceString);
					renderState.blendSource = getBlendFunctionByName(blendSourceString.getCStr());
					DATA_COMPILER_ASSERT(renderState.blendSource != BlendFunction::COUNT
						, compileOptions
						, "Unknown blend function: '%s'"
						, blendSourceString.getCStr()
					);
				}

				if (hasBlendDestination == true)
				{
					RJsonFn::parseString(renderStateJsonObject["blendDestination"], blendDestinationString);
					renderState.blendDestination = getBlendFunctionByName(blendDestinationString.getCStr());
					DATA_COMPILER_ASSERT(renderState.blendDestination != BlendFunction::COUNT
						, compileOptions
						, "Unknown blend function: '%s'"
						, blendDestinationString.getCStr()
					);
				}

				if (hasBlendEquation == true)
				{
					RJsonFn::parseString(renderStateJsonObject["blendEquation"], blendEquationString);
					renderState.blendEquation = getBlendEquationByName(blendEquationString.getCStr());
					DATA_COMPILER_ASSERT(renderState.blendEquation != BlendEquation::COUNT
						, compileOptions
						, "Unknown blend equation: '%s'"
						, blendEquationString.getCStr()
					);
				}

				if (hasCullMode == true)
				{
					RJsonFn::parseString(renderStateJsonObject["cullMode"], cullModeString);
					renderState.cullMode = getCullModeByName(cullModeString.getCStr());
					DATA_COMPILER_ASSERT(renderState.cullMode != CullMode::COUNT
						, compileOptions
						, "Unknown cull mode: '%s'"
						, cullModeString.getCStr()
					);
				}

				if (hasPrimitiveType == true)
				{
					RJsonFn::parseString(renderStateJsonObject["primitiveType"], primitiveTypeString);
					renderState.primitiveType = getPrimitiveTypeByName(primitiveTypeString.getCStr());
					DATA_COMPILER_ASSERT(renderState.primitiveType != PrimitiveType::COUNT
						, compileOptions
						, "Unknown primitive type: '%s'"
						, primitiveTypeString.getCStr()
					);
				}

				DynamicString key(tempAllocator4096);
				key = currentRenderStateJson->pair.first;

				DATA_COMPILER_ASSERT(!MapFn::has(renderStateMap, key)
					, compileOptions
					, "Render state redefined: '%s'"
					, key.getCStr()
				);
				MapFn::set(renderStateMap, key, renderState);
			}
		}

	private:

		void parseSamplerStateList(const char* json)
		{
			TempAllocator4096 tempAllocator4096;
			JsonObject samplerStateListJsonObject(tempAllocator4096);
			RJsonFn::parseObject(json, samplerStateListJsonObject);

			auto currentSamplerStateJson = JsonObjectFn::begin(samplerStateListJsonObject);
			auto endSamplerStateJson = JsonObjectFn::end(samplerStateListJsonObject);

			for (; currentSamplerStateJson != endSamplerStateJson; ++currentSamplerStateJson)
			{
				JsonObject samplerStateJsonObject(tempAllocator4096);
				RJsonFn::parseObject(currentSamplerStateJson->pair.second, samplerStateJsonObject);

				const bool hasFilterMin = JsonObjectFn::has(samplerStateJsonObject, "filterMin");
				const bool hasFilterMag = JsonObjectFn::has(samplerStateJsonObject, "filterMag");
				const bool hasWrapU = JsonObjectFn::has(samplerStateJsonObject, "wrapU");
				const bool hasWrapV = JsonObjectFn::has(samplerStateJsonObject, "wrapV");
				const bool hasWrapW = JsonObjectFn::has(samplerStateJsonObject, "wrapW");

				SamplerState samplerState;
				samplerState.reset();

				DynamicString filterMinString(tempAllocator4096);
				DynamicString filterMagString(tempAllocator4096);
				DynamicString wrapUString(tempAllocator4096);
				DynamicString wrapVString(tempAllocator4096);
				DynamicString wrapWString(tempAllocator4096);

				if (hasFilterMin == true)
				{
					RJsonFn::parseString(samplerStateJsonObject["filterMin"], filterMinString);
					samplerState.filterMin = getSamplerFilterByName(filterMinString.getCStr());
					DATA_COMPILER_ASSERT(samplerState.filterMin != SamplerFilter::COUNT
						, compileOptions
						, "Unknown sampler filter: '%s'"
						, filterMinString.getCStr()
					);
				}

				if (hasFilterMag == true)
				{
					RJsonFn::parseString(samplerStateJsonObject["filterMag"], filterMagString);
					samplerState.filterMag = getSamplerFilterByName(filterMagString.getCStr());
					DATA_COMPILER_ASSERT(samplerState.filterMag != SamplerFilter::COUNT
						, compileOptions
						, "Unknown sampler filter: '%s'"
						, filterMagString.getCStr()
					);
				}

				if (hasWrapU == true)
				{
					RJsonFn::parseString(samplerStateJsonObject["wrapU"], wrapUString);
					samplerState.wrapU = getSamplerWrapByName(wrapUString.getCStr());
					DATA_COMPILER_ASSERT(samplerState.wrapU != SamplerWrap::COUNT
						, compileOptions
						, "Unknown wrap mode: '%s'"
						, wrapUString.getCStr()
					);
				}

				if (hasWrapV == true)
				{
					RJsonFn::parseString(samplerStateJsonObject["wrapV"], wrapVString);
					samplerState.wrapV = getSamplerWrapByName(wrapVString.getCStr());
					DATA_COMPILER_ASSERT(samplerState.wrapV != SamplerWrap::COUNT
						, compileOptions
						, "Unknown wrap mode: '%s'"
						, wrapVString.getCStr()
					);
				}

				if (hasWrapW == true)
				{
					RJsonFn::parseString(samplerStateJsonObject["wrapW"], wrapWString);
					samplerState.wrapW = getSamplerWrapByName(wrapWString.getCStr());
					DATA_COMPILER_ASSERT(samplerState.wrapW != SamplerWrap::COUNT
						, compileOptions
						, "Unknown wrap mode: '%s'"
						, wrapWString.getCStr()
					);
				}

				DynamicString key(tempAllocator4096);
				key = currentSamplerStateJson->pair.first;

				DATA_COMPILER_ASSERT(!MapFn::has(samplerStateMap, key)
					, compileOptions
					, "Sampler state redefined: '%s'"
					, key.getCStr()
				);
				MapFn::set(samplerStateMap, key, samplerState);
			}
		}

	private:

		void parseRioRendererShaders(const char* json)
		{
			TempAllocator4096 tempAllocator4096;
			JsonObject rioRendererShaderListJsonObject(tempAllocator4096);
			RJsonFn::parseObject(json, rioRendererShaderListJsonObject);

			auto currentShaderListJsonObject = JsonObjectFn::begin(rioRendererShaderListJsonObject);
			auto endShaderListJsonObject = JsonObjectFn::end(rioRendererShaderListJsonObject);
			for (; currentShaderListJsonObject != endShaderListJsonObject; ++currentShaderListJsonObject)
			{
				JsonObject shaderJsonObject(tempAllocator4096);
				RJsonFn::parseObject(currentShaderListJsonObject->pair.second, shaderJsonObject);

				RioRendererShader rioRendererShader(getDefaultAllocator());
				if (JsonObjectFn::has(shaderJsonObject, "includeList"))
				{
					RJsonFn::parseString(shaderJsonObject["includeList"], rioRendererShader.includeListString);
				}
				if (JsonObjectFn::has(shaderJsonObject, "code"))
				{
					RJsonFn::parseVerbatim(shaderJsonObject["code"], rioRendererShader.codeString);
				}
				if (JsonObjectFn::has(shaderJsonObject, "vertexShaderCode"))
				{
					RJsonFn::parseVerbatim(shaderJsonObject["vertexShaderCode"], rioRendererShader.vertexShaderCode);
				}
				if (JsonObjectFn::has(shaderJsonObject, "fragmentShaderCode"))
				{
					RJsonFn::parseVerbatim(shaderJsonObject["fragmentShaderCode"], rioRendererShader.fragmentShaderCodeString);
				}
				if (JsonObjectFn::has(shaderJsonObject, "varying"))
				{
					RJsonFn::parseVerbatim(shaderJsonObject["varying"], rioRendererShader.varyingString);
				}
				if (JsonObjectFn::has(shaderJsonObject, "vertexShaderInputOutput"))
				{
					RJsonFn::parseVerbatim(shaderJsonObject["vertexShaderInputOutput"], rioRendererShader.vertexShaderInputOutput);
				}
				if (JsonObjectFn::has(shaderJsonObject, "fragmentShaderInputOutput"))
				{
					RJsonFn::parseVerbatim(shaderJsonObject["fragmentShaderInputOutput"], rioRendererShader.fragmentShaderInputOutput);
				}
				if (JsonObjectFn::has(shaderJsonObject, "samplerList"))
				{
					parseRioRendererSamplerList(shaderJsonObject["samplerList"], rioRendererShader);
				}

				DynamicString key(tempAllocator4096);
				key = currentShaderListJsonObject->pair.first;

				DATA_COMPILER_ASSERT(!MapFn::has(rioRendererShaderMap, key)
					, compileOptions
					, "RioRenderer shader redefined: '%s'"
					, key.getCStr()
				);
				MapFn::set(rioRendererShaderMap, key, rioRendererShader);
			}
		}

	private:

		void parseRioRendererSamplerList(const char* json, RioRendererShader& rioRendererShader)
		{
			TempAllocator4096 tempAllocator4096;
			JsonObject rioRendererSamplerListJsonObject(tempAllocator4096);
			RJsonFn::parseObject(json, rioRendererSamplerListJsonObject);

			auto currentSamplerJson = JsonObjectFn::begin(rioRendererSamplerListJsonObject);
			auto endSamplerJson = JsonObjectFn::end(rioRendererSamplerListJsonObject);
			for (; currentSamplerJson != endSamplerJson; ++currentSamplerJson)
			{
				JsonObject samplerJsonObject(tempAllocator4096);
				RJsonFn::parseObject(currentSamplerJson->pair.second, samplerJsonObject);

				DynamicString samplerStateStringKey(tempAllocator4096);
				RJsonFn::parseString(samplerJsonObject["samplerState"], samplerStateStringKey);

				DATA_COMPILER_ASSERT(MapFn::has(samplerStateMap, samplerStateStringKey)
					, compileOptions
					, "Unknown sampler state: '%s'"
					, samplerStateStringKey.getCStr()
				);

				DynamicString key(tempAllocator4096);
				key = currentSamplerJson->pair.first;

				DATA_COMPILER_ASSERT(!MapFn::has(rioRendererShader.samplerMap, key)
					, compileOptions
					, "RioRenderer sampler redefined: '%s'"
					, key.getCStr()
				);
				MapFn::set(rioRendererShader.samplerMap, key, samplerStateStringKey);
			}
		}

	private:

		void parseShaderList(const char* json)
		{
			TempAllocator4096 tempAllocator4096;
			JsonObject shaderListJsonObject(tempAllocator4096);
			RJsonFn::parseObject(json, shaderListJsonObject);

			auto currentShaderJson = JsonObjectFn::begin(shaderListJsonObject);
			auto endShaderJson = JsonObjectFn::end(shaderListJsonObject);
			for (; currentShaderJson != endShaderJson; ++currentShaderJson)
			{
				JsonObject jsonObject(tempAllocator4096);
				RJsonFn::parseObject(currentShaderJson->pair.second, jsonObject);

				ShaderPermutation shader(getDefaultAllocator());
				RJsonFn::parseString(jsonObject["rioRendererShader"], shader.rioRendererShaderName);
				RJsonFn::parseString(jsonObject["renderState"], shader.renderStateString);

				DynamicString key(tempAllocator4096);
				key = currentShaderJson->pair.first;

				DATA_COMPILER_ASSERT(!MapFn::has(shaderPermutationMap, key)
					, compileOptions
					, "Shader redefined: '%s'"
					, key.getCStr()
				);
				MapFn::set(shaderPermutationMap, key, shader);
			}
		}

	private:

		void parseStaticCompile(const char* json)
		{
			TempAllocator4096 tempAllocator4096;
			JsonArray staticCompileJsonArray(tempAllocator4096);
			RJsonFn::parseArray(json, staticCompileJsonArray);

			for (uint32_t i = 0; i < ArrayFn::getCount(staticCompileJsonArray); ++i)
			{
				JsonObject staticCompileJsonObject(tempAllocator4096);
				RJsonFn::parseObject(staticCompileJsonArray[i], staticCompileJsonObject);

				StaticCompile staticCompile(getDefaultAllocator());
				RJsonFn::parseString(staticCompileJsonObject["shader"], staticCompile.shaderString);

				JsonArray defineListJsonArray(tempAllocator4096);
				RJsonFn::parseArray(staticCompileJsonObject["defineList"], defineListJsonArray);
				for (uint32_t i = 0; i < ArrayFn::getCount(defineListJsonArray); ++i)
				{
					DynamicString defineString(tempAllocator4096);
					RJsonFn::parseString(defineListJsonArray[i], defineString);
					VectorFn::pushBack(staticCompile.defineList, defineString);
				}

				VectorFn::pushBack(staticCompileList, staticCompile);
			}
		}

	private:

		void deleteTemporaryFiles()
		{
			const char* vertexShaderSourcePath = this->vertexShaderSourcePath.getCStr();
			const char* fragmentShaderSourcePath = this->fragmentShaderSourcePath.getCStr();
			const char* varyingPath = this->varyingPath.getCStr();
			const char* vertexShaderCompiledPath = this->vertexShaderCompiledPath.getCStr();
			const char* fragmentShaderCompiledPath = this->fragmentShaderCompiledPath.getCStr();

			if (compileOptions.doesFileExist(vertexShaderSourcePath))
			{
				compileOptions.deleteFile(vertexShaderSourcePath);
			}

			if (compileOptions.doesFileExist(fragmentShaderSourcePath))
			{
				compileOptions.deleteFile(fragmentShaderSourcePath);
			}

			if (compileOptions.doesFileExist(varyingPath))
			{
				compileOptions.deleteFile(varyingPath);
			}

			if (compileOptions.doesFileExist(vertexShaderCompiledPath))
			{
				compileOptions.deleteFile(vertexShaderCompiledPath);
			}

			if (compileOptions.doesFileExist(fragmentShaderCompiledPath))
			{
				compileOptions.deleteFile(fragmentShaderCompiledPath);
			}
		}

	public:

		void compile()
		{
			compileOptions.write(RESOURCE_VERSION_SHADER);
			compileOptions.write(VectorFn::getCount(staticCompileList));

			for (uint32_t i = 0; i < VectorFn::getCount(staticCompileList); ++i)
			{
				const StaticCompile& staticCompile = staticCompileList[i];
				const DynamicString& shaderString = staticCompile.shaderString;
				const Vector<DynamicString>& defineList = staticCompile.defineList;

				TempAllocator1024 tempAllocator1024;
				DynamicString shaderStringCombined(tempAllocator1024);
				shaderStringCombined = shaderString;
				for (uint32_t i = 0; i < VectorFn::getCount(defineList); ++i)
				{
					shaderStringCombined += "+";
					shaderStringCombined += defineList[i];
				}
				const StringId32 shaderStringIndex(shaderStringCombined.getCStr());

				DATA_COMPILER_ASSERT(MapFn::has(shaderPermutationMap, staticCompile.shaderString)
					, compileOptions
					, "Unknown shader: '%s'"
					, shaderString.getCStr()
				);

				const ShaderPermutation& shaderPermutation = shaderPermutationMap[shaderString];
				const DynamicString& rioRendererShaderName = shaderPermutation.rioRendererShaderName;
				const DynamicString& renderStateString = shaderPermutation.renderStateString;

				DATA_COMPILER_ASSERT(MapFn::has(rioRendererShaderMap, shaderPermutation.rioRendererShaderName)
					, compileOptions
					, "Unknown RioRenderer shader: '%s'"
					, rioRendererShaderName.getCStr()
				);

				DATA_COMPILER_ASSERT(MapFn::has(renderStateMap, shaderPermutation.renderStateString)
					, compileOptions
					, "Unknown render state: '%s'"
					, renderStateString.getCStr()
				);

				const RenderState& renderState = renderStateMap[renderStateString];

				compileOptions.write(shaderStringIndex.id); // Shader name
				compileOptions.write(renderState.encode()); // Render state

				compileSamplerStateList(rioRendererShaderName.getCStr()); // Sampler states
				
				compile(rioRendererShaderName.getCStr(), defineList); // Shader code
			}
		}

	private:

		void compileSamplerStateList(const char* rioRendererShaderName)
		{
			TempAllocator512 tempAllocator512;
			DynamicString key(tempAllocator512);
			key = rioRendererShaderName;
			const RioRendererShader& rioRendererShader = rioRendererShaderMap[key];

			compileOptions.write(MapFn::getCount(rioRendererShader.samplerMap));

			auto currentSamplerEntryPair = MapFn::begin(rioRendererShader.samplerMap);
			auto endSamplerEntryPair = MapFn::end(rioRendererShader.samplerMap);
			for (; currentSamplerEntryPair != endSamplerEntryPair; ++currentSamplerEntryPair)
			{
				const DynamicString& name = currentSamplerEntryPair->pair.first;
				const DynamicString& samplerStateStringKey = currentSamplerEntryPair->pair.second;
				const SamplerState& samplerState = samplerStateMap[samplerStateStringKey];

				compileOptions.write(name.toStringId32());
				compileOptions.write(samplerState.encode());
			}
		}

	public:

		void compile(const char* rioRendererShaderName, const Vector<DynamicString>& defineList)
		{
			TempAllocator512 tempAllocator512;
			DynamicString key(tempAllocator512);
			key = rioRendererShaderName;
			const RioRendererShader& rioRendererShader = rioRendererShaderMap[key];

			DynamicString includedCodeString(getDefaultAllocator());
			if (!(rioRendererShader.includeListString == ""))
			{
				const RioRendererShader& includedRioRendererShader = rioRendererShaderMap[rioRendererShader.includeListString];
				includedCodeString = includedRioRendererShader.codeString;
			}

			StringStream vertexShaderCode(getDefaultAllocator());
			StringStream fragmentShaderCode(getDefaultAllocator());
			vertexShaderCode << rioRendererShader.vertexShaderInputOutput.getCStr();

			for (uint32_t i = 0; i < VectorFn::getCount(defineList); ++i)
			{
				vertexShaderCode << "#define " << defineList[i].getCStr() << "\n";
			}

			vertexShaderCode << includedCodeString.getCStr();
			vertexShaderCode << rioRendererShader.codeString.getCStr();
			vertexShaderCode << rioRendererShader.vertexShaderCode.getCStr();
			fragmentShaderCode << rioRendererShader.fragmentShaderInputOutput.getCStr();

			for (uint32_t i = 0; i < VectorFn::getCount(defineList); ++i)
			{
				fragmentShaderCode << "#define " << defineList[i].getCStr() << "\n";
			}

			fragmentShaderCode << includedCodeString.getCStr();
			fragmentShaderCode << rioRendererShader.codeString.getCStr();
			fragmentShaderCode << rioRendererShader.fragmentShaderCodeString.getCStr();

			compileOptions.writeTemporary(this->vertexShaderSourcePath.getCStr(), vertexShaderCode);
			compileOptions.writeTemporary(this->fragmentShaderSourcePath.getCStr(), fragmentShaderCode);
			compileOptions.writeTemporary(this->varyingPath.getCStr(), rioRendererShader.varyingString.getCStr(), rioRendererShader.varyingString.getLength());

			TempAllocator4096 tempAllocator4096;
			StringStream output(tempAllocator4096);

			int32_t externalCompilerResult = runExternalCompiler(compileOptions, vertexShaderSourcePath.getCStr()
				, this->vertexShaderCompiledPath.getCStr()
				, this->varyingPath.getCStr()
				, "vertex"
				, compileOptions.getPlatformName()
				, output
			);

			if (externalCompilerResult != 0)
			{
				deleteTemporaryFiles();
				DATA_COMPILER_ASSERT(false
					, compileOptions
					, "Failed to compile vertex shader:\n%s"
					, StringStreamFn::getCStr(output)
				);
			}

			ArrayFn::clear(output);
			externalCompilerResult = runExternalCompiler(compileOptions, this->fragmentShaderSourcePath.getCStr()
				, this->fragmentShaderCompiledPath.getCStr()
				, this->varyingPath.getCStr()
				, "fragment"
				, compileOptions.getPlatformName()
				, output
			);

			if (externalCompilerResult != 0)
			{
				deleteTemporaryFiles();
				DATA_COMPILER_ASSERT(false
					, compileOptions
					, "Failed to compile fragment shader:\n%s"
					, StringStreamFn::getCStr(output)
				);
			}

			Buffer vertexShaderTemporary = compileOptions.readTemporary(this->vertexShaderCompiledPath.getCStr());
			Buffer fragmentShaderTemporary = compileOptions.readTemporary(this->fragmentShaderCompiledPath.getCStr());

			deleteTemporaryFiles();

			// Write
			compileOptions.write(ArrayFn::getCount(vertexShaderTemporary));
			compileOptions.write(vertexShaderTemporary);
			compileOptions.write(ArrayFn::getCount(fragmentShaderTemporary));
			compileOptions.write(fragmentShaderTemporary);
		}
	};

} // namespace ShaderResourceInternalFn

} // namespace Rio