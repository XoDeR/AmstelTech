#pragma once

#include "Core/Containers/Array.h"
#include "Core/Containers/Types.h"
#include "Core/FileSystem/Types.h"
#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

#include "RioRenderer/RioRenderer.h"

#include "Resource/MeshResource.h"

#include "Resource/CompileOptions.h"

#include "Core/Containers/Map.h"
#include "Core/Containers/Vector.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/FileSystem/ReaderWriter.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"

#include "Device/DeviceLog.h"

#include "Resource/ResourceManager.h"

namespace Rio
{
	
namespace MeshResourceInternalFn
{

struct MeshCompiler
{
	CompileOptions& compileOptions;

	Array<float> positionList;
	Array<float> normalList;
	Array<float> uvList;

	Array<uint16_t> positionIndexList;
	Array<uint16_t> normalIndexList;
	Array<uint16_t> uvIndexList;

	Matrix4x4 matrixLocal = MATRIX4X4_IDENTITY;

	uint32_t vertexStride = 0;
	Array<char> vertexBuffer;
	Array<uint16_t> indexBuffer;

	Aabb aabb;
	Obb obb;

	RioRenderer::VertexDecl vertexDecl;

	bool hasNormal = false;
	bool hasUv = false;

	MeshCompiler(CompileOptions& compileOptions)
		: compileOptions(compileOptions)
		, positionList(getDefaultAllocator())
		, normalList(getDefaultAllocator())
		, uvList(getDefaultAllocator())
		, positionIndexList(getDefaultAllocator())
		, normalIndexList(getDefaultAllocator())
		, uvIndexList(getDefaultAllocator())
		, vertexBuffer(getDefaultAllocator())
		, indexBuffer(getDefaultAllocator())
	{
	}

	void reset()
	{
		ArrayFn::clear(positionList);
		ArrayFn::clear(normalList);
		ArrayFn::clear(uvList);

		ArrayFn::clear(positionIndexList);
		ArrayFn::clear(normalIndexList);
		ArrayFn::clear(uvIndexList);

		vertexStride = 0;
		ArrayFn::clear(vertexBuffer);
		ArrayFn::clear(indexBuffer);

		AabbFn::reset(aabb);
		memset(&obb, 0, sizeof(obb));
		memset(&vertexDecl, 0, sizeof(vertexDecl));

		hasNormal = false;
		hasUv = false;
	}

	void parse(const char* geometry, const char* node)
	{
		TempAllocator4096 tempAllocator4096;
		JsonObject jsonObject(tempAllocator4096);
		JsonObject nodeJsonObject(tempAllocator4096);
		RJsonFn::parse(geometry, jsonObject);
		RJsonFn::parse(node, nodeJsonObject);

		hasNormal = JsonObjectFn::has(jsonObject, "normal");
		hasUv = JsonObjectFn::has(jsonObject, "textureCoord");

		parseFloatJsonArray(jsonObject["position"], positionList);

		if (hasNormal == true)
		{
			parseFloatJsonArray(jsonObject["normal"], normalList);
		}
		if (hasUv == true)
		{
			parseFloatJsonArray(jsonObject["textureCoord"], uvList);
		}

		parseIndexList(jsonObject["indexList"]);

		matrixLocal = RJsonFn::parseMatrix4x4(nodeJsonObject["matrixLocal"]);
	}

private:

	void parseFloatJsonArray(const char* jsonArrayString, Array<float>& outputFloatArray)
	{
		TempAllocator4096 tempAllocator4096;
		JsonArray jsonArray(tempAllocator4096);
		RJsonFn::parseArray(jsonArrayString, jsonArray);

		ArrayFn::resize(outputFloatArray, ArrayFn::getCount(jsonArray));
		for (uint32_t i = 0; i < ArrayFn::getCount(jsonArray); ++i)
		{
			outputFloatArray[i] = RJsonFn::parseFloat(jsonArray[i]);
		}
	}

private:

	void parseIndexJsonArray(const char* jsonArrayString, Array<uint16_t>& outputIndexArray)
	{
		TempAllocator4096 tempAllocator4096;
		JsonArray jsonArray(tempAllocator4096);
		RJsonFn::parseArray(jsonArrayString, jsonArray);

		ArrayFn::resize(outputIndexArray, ArrayFn::getCount(jsonArray));
		for (uint32_t i = 0; i < ArrayFn::getCount(jsonArray); ++i)
		{
			outputIndexArray[i] = (uint16_t)RJsonFn::parseInt32(jsonArray[i]);
		}
	}

private:

	void parseIndexList(const char* json)
	{
		TempAllocator4096 tempAllocator4096;
		JsonObject jsonObject(tempAllocator4096);
		RJsonFn::parse(json, jsonObject);

		JsonArray dataJsonArray(tempAllocator4096);
		RJsonFn::parseArray(jsonObject["data"], dataJsonArray);

		parseIndexJsonArray(dataJsonArray[0], positionIndexList);

		if (hasNormal == true)
		{
			parseIndexJsonArray(dataJsonArray[1], normalIndexList);
		}
		if (hasUv == true)
		{
			parseIndexJsonArray(dataJsonArray[2], uvIndexList);
		}
	}

public:

	void compile()
	{
		vertexStride = 0;
		vertexStride += 3 * sizeof(float);
		vertexStride += (hasNormal ? 3 * sizeof(float) : 0);
		vertexStride += (hasUv ? 2 * sizeof(float) : 0);

		// Generate vertex/index buffer
		ArrayFn::resize(indexBuffer, ArrayFn::getCount(positionIndexList));

		uint16_t index = 0;
		for (uint32_t i = 0; i < ArrayFn::getCount(positionIndexList); ++i)
		{
			indexBuffer[i] = index++;

			const uint16_t positionIndex = positionIndexList[i] * 3;
			Vector3 xyz;
			xyz.x = positionList[positionIndex + 0];
			xyz.y = positionList[positionIndex + 1];
			xyz.z = positionList[positionIndex + 2];
			xyz = xyz * matrixLocal;
			ArrayFn::push(vertexBuffer, (char*)&xyz, sizeof(xyz));

			if (hasNormal == true)
			{
				const uint16_t normalIndex = normalIndexList[i] * 3;
				Vector3 normal;
				normal.x = normalList[normalIndex + 0];
				normal.y = normalList[normalIndex + 1];
				normal.z = normalList[normalIndex + 2];
				ArrayFn::push(vertexBuffer, (char*)&normal, sizeof(normal));
			}
			if (hasUv == true)
			{
				const uint16_t uvIndex = uvIndexList[i] * 2;
				Vector2 uv;
				uv.x = uvList[uvIndex + 0];
				uv.y = uvList[uvIndex + 1];
				ArrayFn::push(vertexBuffer, (char*)&uv, sizeof(uv));
			}
		}

		// Vertex decl
		vertexDecl.begin();
		vertexDecl.add(RioRenderer::Attrib::Position, 3, RioRenderer::AttribType::Float);

		if (hasNormal == true)
		{
			vertexDecl.add(RioRenderer::Attrib::Normal, 3, RioRenderer::AttribType::Float, true);
		}
		if (hasUv == true)
		{
			vertexDecl.add(RioRenderer::Attrib::TexCoord0, 2, RioRenderer::AttribType::Float);
		}

		vertexDecl.end();

		// Bounds
		AabbFn::reset(aabb);
		AabbFn::addPoints(aabb
			, ArrayFn::getCount(positionList) / 3
			, sizeof(float) * 3
			, ArrayFn::begin(positionList)
			);

		aabb = AabbFn::getTransformedByMatrix(aabb, matrixLocal);

		obb.transformMatrix = createMatrix4x4(QUATERNION_IDENTITY, AabbFn::getCenter(aabb));
		obb.halfExtents.x = (aabb.max.x - aabb.min.x) * 0.5f;
		obb.halfExtents.y = (aabb.max.y - aabb.min.y) * 0.5f;
		obb.halfExtents.z = (aabb.max.z - aabb.min.z) * 0.5f;
	}

	void write()
	{
		compileOptions.write(vertexDecl);
		compileOptions.write(obb);

		compileOptions.write(ArrayFn::getCount(vertexBuffer) / vertexStride);
		compileOptions.write(vertexStride);
		compileOptions.write(ArrayFn::getCount(indexBuffer));

		compileOptions.write(vertexBuffer);
		compileOptions.write(ArrayFn::begin(indexBuffer), ArrayFn::getCount(indexBuffer) * sizeof(uint16_t));
	}
};

} // namespace MeshResourceInternalFn

} // namespace Rio	
	