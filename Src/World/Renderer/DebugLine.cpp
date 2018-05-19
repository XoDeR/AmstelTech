#include "World/Renderer/DebugLine.h"

#include "Core/Math/Color4.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Intersection.h"
#include "Core/Math/Math.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"

#include "Device/Pipeline.h"

#include "Resource/ResourceManager.h"
#include "Resource/Renderer/MeshResource.h"
#include "Resource/Sprite/SpriteResource.h"
#include "Resource/UnitResource.h"

#include "World/Renderer/ShaderManager.h"

#include <cstring> // memcpy

namespace Rio
{

DebugLine::DebugLine(ShaderManager& shaderManager, bool enableDepthTest)
	: shaderManager(&shaderManager)
	, shaderString(enableDepthTest ? "debug_line" : "debug_line_noz")
{
	this->vertexDecl.begin()
		.add(RioRenderer::Attrib::Position, 3, RioRenderer::AttribType::Float)
		.add(RioRenderer::Attrib::Color0,   4, RioRenderer::AttribType::Uint8, true)
		.end();
}

DebugLine::~DebugLine()
{
	marker = 0;
}

void DebugLine::addLine(const Vector3& start, const Vector3& end, const Color4& color)
{
	if (this->lineListCount >= LINE_LIST_COUNT_MAX)
	{
		return;
	}

	lineList[this->lineListCount].p0 = start;
	lineList[this->lineListCount].c0 = getPackedAbgr(color);
	lineList[this->lineListCount].p1 = end;
	lineList[this->lineListCount].c1 = getPackedAbgr(color);

	++this->lineListCount;
}

void DebugLine::addAxisList(const Matrix4x4& m, float length)
{
	const Vector3 position = getTranslation(m);
	addLine(position, position + getXAxis(m)*length, COLOR4_RED);
	addLine(position, position + getYAxis(m)*length, COLOR4_GREEN);
	addLine(position, position + getZAxis(m)*length, COLOR4_BLUE);
}

void DebugLine::addArc(const Vector3& center, float radius, const Vector3& planeNormal, const Vector3& midPointNormal, const Color4& color, uint32_t circleSegmentListCount)
{
	const Vector3 x = midPointNormal * radius;
	const Vector3 y = getCrossProduct(midPointNormal, planeNormal) * radius;
	const uint32_t circleSegmentListCountToDraw = circleSegmentListCount / 2;
	const float step = PI / (float)(circleSegmentListCountToDraw > 3 ? circleSegmentListCountToDraw : 3);
	Vector3 from = center - y;

	for (uint32_t i = 0; i <= circleSegmentListCountToDraw; ++i)
	{
		const float t = step * i - PI_HALF;
		const Vector3 to = center + x  *getCosFloat(t) + y * getSinFloat(t);
		addLine(from, to, color);
		from = to;
	}
}

void DebugLine::addCircle(const Vector3& center, float radius, const Vector3& normal, const Color4& color, uint32_t circleSegmentListCount)
{
	const Vector3 directionList[] =
	{
		{ normal.z,				normal.z, -normal.x - normal.y	},
		{ -normal.y - normal.z, normal.x, normal.x				}
	};

	const uint32_t index = ((normal.z != 0.0f) && (-normal.x != normal.y));
	Vector3 right = directionList[index];
	getNormalized(right);

	const Vector3 x = right * radius;
	const Vector3 y = getCrossProduct(right, normal) * radius;
	const float step = PI_TWO / (float)(circleSegmentListCount > 3 ? circleSegmentListCount : 3);
	Vector3 from = center - y;

	for (uint32_t i = 0; i <= circleSegmentListCount; ++i)
	{
		const float t = step * i - PI_HALF;
		const Vector3 to = center + x * getCosFloat(t) + y * getSinFloat(t);
		addLine(from, to, color);
		from = to;
	}
}

void DebugLine::addCone(const Vector3& baseCenter, const Vector3& tip, float radius, const Color4& color, uint32_t circleSegmentListCount)
{
	Vector3 normal = tip - baseCenter;
	getNormalized(normal);
	
	const Vector3 directionList[] =
	{
		{ normal.z,				normal.z, -normal.x - normal.y	},
		{ -normal.y - normal.z, normal.x, normal.x				}
	};

	const uint32_t index = ((normal.z != 0.0f) && (-normal.x != normal.y));
	Vector3 right = directionList[index];
	getNormalized(right);

	const Vector3 x = right * radius;
	const Vector3 y = getCrossProduct(right, normal) * radius;
	const float step = PI_TWO / (float)(circleSegmentListCount > 3 ? circleSegmentListCount : 3);
	Vector3 from = baseCenter - y;

	for (uint32_t i = 0; i <= circleSegmentListCount; ++i)
	{
		const float t = step * i - PI_HALF;
		const Vector3 to = baseCenter + x * getCosFloat(t) + y * getSinFloat(t);
		addLine(from, to, color);
		addLine(from, tip, color);
		from = to;
	}
}

void DebugLine::addSphere(const Vector3& center, const float radius, const Color4& color, uint32_t circleSegmentListCount)
{
	addCircle(center, radius, VECTOR3_XAXIS, color, circleSegmentListCount);
	addCircle(center, radius, VECTOR3_YAXIS, color, circleSegmentListCount);
	addCircle(center, radius, VECTOR3_ZAXIS, color, circleSegmentListCount);
}

void DebugLine::addFrustum(const Matrix4x4& modelViewProjectionMatrix4x4, const Color4& color)
{
	Frustum frustum;
	FrustumFn::createFromMatrix(frustum, modelViewProjectionMatrix4x4);

	Vector3 pointList[8];

	getPlanesIntersection(frustum.planeNear, frustum.planeLeft, frustum.planeTop, pointList[0]);
	getPlanesIntersection(frustum.planeNear, frustum.planeTop, frustum.planeRight, pointList[1]);
	getPlanesIntersection(frustum.planeNear, frustum.planeRight, frustum.planeBottom, pointList[2]);
	getPlanesIntersection(frustum.planeNear, frustum.planeBottom, frustum.planeLeft, pointList[3]);
	getPlanesIntersection(frustum.planeFar, frustum.planeLeft, frustum.planeTop, pointList[4]);
	getPlanesIntersection(frustum.planeFar, frustum.planeTop, frustum.planeRight, pointList[5]);
	getPlanesIntersection(frustum.planeFar, frustum.planeRight, frustum.planeBottom, pointList[6]);
	getPlanesIntersection(frustum.planeFar, frustum.planeBottom, frustum.planeLeft, pointList[7]);

	addLine(pointList[0], pointList[1], color);
	addLine(pointList[1], pointList[2], color);
	addLine(pointList[2], pointList[3], color);
	addLine(pointList[3], pointList[0], color);
	addLine(pointList[4], pointList[5], color);
	addLine(pointList[5], pointList[6], color);
	addLine(pointList[6], pointList[7], color);
	addLine(pointList[7], pointList[4], color);
	addLine(pointList[0], pointList[4], color);
	addLine(pointList[1], pointList[5], color);
	addLine(pointList[2], pointList[6], color);
	addLine(pointList[3], pointList[7], color);
}

void DebugLine::addObb(const Matrix4x4& transformMatrix, const Vector3& halfExtents, const Color4& color)
{
	const Vector3 o = createVector3(transformMatrix.t.x, transformMatrix.t.y, transformMatrix.t.z);
	const Vector3 x = createVector3(transformMatrix.x.x, transformMatrix.x.y, transformMatrix.x.z) * halfExtents.x;
	const Vector3 y = createVector3(transformMatrix.y.x, transformMatrix.y.y, transformMatrix.y.z) * halfExtents.y;
	const Vector3 z = createVector3(transformMatrix.z.x, transformMatrix.z.y, transformMatrix.z.z) * halfExtents.z;

	// Back face
	addLine(o - x - y - z, o + x - y - z, color);
	addLine(o + x - y - z, o + x + y - z, color);
	addLine(o + x + y - z, o - x + y - z, color);
	addLine(o - x + y - z, o - x - y - z, color);

	addLine(o - x - y + z, o + x - y + z, color);
	addLine(o + x - y + z, o + x + y + z, color);
	addLine(o + x + y + z, o - x + y + z, color);
	addLine(o - x + y + z, o - x - y + z, color);

	addLine(o - x - y - z, o - x - y + z, color);
	addLine(o + x - y - z, o + x - y + z, color);
	addLine(o + x + y - z, o + x + y + z, color);
	addLine(o - x + y - z, o - x + y + z, color);
}

void DebugLine::addMesh(const Matrix4x4& transformMatrix, const void* vertexList, uint32_t stride, const uint16_t* indexList, uint32_t count, const Color4& color)
{
	for (uint32_t i = 0; i < count; i += 3)
	{
		const uint32_t i0 = indexList[i + 0];
		const uint32_t i1 = indexList[i + 1];
		const uint32_t i2 = indexList[i + 2];

		const Vector3& v0 = *(const Vector3*)((const char*)vertexList + i0 * stride) * transformMatrix;
		const Vector3& v1 = *(const Vector3*)((const char*)vertexList + i1 * stride) * transformMatrix;
		const Vector3& v2 = *(const Vector3*)((const char*)vertexList + i2 * stride) * transformMatrix;

		addLine(v0, v1, color);
		addLine(v1, v2, color);
		addLine(v2, v0, color);
	}
}

void DebugLine::addUnit(ResourceManager& resourceManager, const Matrix4x4& transformMatrix, StringId64 name, const Color4& color)
{
	const UnitResource& unitResource = *(const UnitResource*)resourceManager.getResourceData(RESOURCE_TYPE_UNIT, name);

	const char* componentDataBufferPointer = (const char*)(&unitResource + 1);

	for (uint32_t componentCounter = 0; componentCounter < unitResource.componentTypeListCount; ++componentCounter)
	{
		const ComponentData* componentData = (const ComponentData*)componentDataBufferPointer;
		const uint32_t* unitIndexList = (const uint32_t*)(componentData + 1);
		const char* data = (const char*)(unitIndexList + componentData->instanceListCount);

		if (componentData->type == COMPONENT_TYPE_MESH_RENDERER)
		{
			const MeshRendererDesc* meshRendererDesc = (const MeshRendererDesc*)data;
			for (uint32_t i = 0; i < componentData->instanceListCount; ++i, ++meshRendererDesc)
			{
				const MeshResource* meshResource = (const MeshResource*)resourceManager.getResourceData(RESOURCE_TYPE_MESH, meshRendererDesc->meshResource);
				const MeshGeometry* meshGeometry = meshResource->getMeshGeometry(meshRendererDesc->geometryName);

				addMesh(transformMatrix
					, meshGeometry->vertexData.data
					, meshGeometry->vertexData.stride
					, (uint16_t*)meshGeometry->indexData.data
					, meshGeometry->indexData.indexListCount
					, color
					);
			}
		}
		else if (componentData->type == COMPONENT_TYPE_SPRITE_RENDERER)
		{
			const SpriteRendererDesc* spriteRendererDesc = (const SpriteRendererDesc*)data;
			for (uint32_t i = 0; i < componentData->instanceListCount; ++i, ++spriteRendererDesc)
			{
				const SpriteResource* spriteResource = (const SpriteResource*)resourceManager.getResourceData(RESOURCE_TYPE_SPRITE, spriteRendererDesc->spriteResource);

				addObb(spriteResource->obb.transformMatrix * transformMatrix
					, spriteResource->obb.halfExtents
					, color
					);
			}
		}

		componentDataBufferPointer += componentData->size + sizeof(ComponentData);
	}
}

void DebugLine::reset()
{
	this->lineListCount = 0;
}

void DebugLine::submit()
{
	if (!this->lineListCount)
	{
		return;
	}

	if (!RioRenderer::getAvailableTransientVertexBuffer(this->lineListCount * 2, this->vertexDecl))
	{
		return;
	}

	RioRenderer::TransientVertexBuffer transientVertexBuffer;
	RioRenderer::allocTransientVertexBuffer(&transientVertexBuffer, this->lineListCount * 2, this->vertexDecl);
	memcpy(transientVertexBuffer.data, lineList, sizeof(Line) * this->lineListCount);

	RioRenderer::setVertexBuffer(0, &transientVertexBuffer, 0, this->lineListCount * 2);
	shaderManager->submit(this->shaderString, VIEW_DEBUG);
}

} // namespace Rio
