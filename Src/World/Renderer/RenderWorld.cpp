#include "World/Renderer/RenderWorld.h"

#include "Core/Containers/HashMap.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Color4.h"
#include "Core/Math/Intersection.h"
#include "Core/Math/Matrix4x4.h"

#include "Device/Pipeline.h"

#include "Resource/Renderer/MeshResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/Sprite/SpriteResource.h"

#include "World/Renderer/DebugLine.h"

#include "World/Renderer/Material.h"
#include "World/Renderer/MaterialManager.h"

#include "World/UnitManager.h"

namespace Rio
{

static void unitDestroyedCallbackBridge(UnitId unitId, void* userPtr)
{
	((RenderWorld*)userPtr)->unitDestroyedCallback(unitId);
}

RenderWorld::RenderWorld(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager)
	: allocator(&a)
	, resourceManager(&resourceManager)
	, shaderManager(&shaderManager)
	, materialManager(&materialManager)
	, unitManager(&unitManager)
	, meshManager(a)
	, spriteManager(a)
	, lightManager(a)
{
	unitManager.registerDestroyFunction(unitDestroyedCallbackBridge, this);

	this->uniformLightPosition = RioRenderer::createUniform("u_light_position", RioRenderer::UniformType::Vec4);
	this->uniformLightDirection = RioRenderer::createUniform("u_light_direction", RioRenderer::UniformType::Vec4);
	this->uniformLightColor = RioRenderer::createUniform("u_light_color", RioRenderer::UniformType::Vec4);
	this->uniformLightRange = RioRenderer::createUniform("u_light_range", RioRenderer::UniformType::Vec4);
	this->uniformLightIntensity = RioRenderer::createUniform("u_light_intensity", RioRenderer::UniformType::Vec4);
}

RenderWorld::~RenderWorld()
{
	unitManager->unregisterDestroyFunction(this);

	RioRenderer::destroy(this->uniformLightIntensity);
	RioRenderer::destroy(this->uniformLightRange);
	RioRenderer::destroy(this->uniformLightColor);
	RioRenderer::destroy(this->uniformLightDirection);
	RioRenderer::destroy(this->uniformLightPosition);

	meshManager.destroy();
	spriteManager.destroy();
	lightManager.destroy();

	marker = 0;
}

MeshInstance RenderWorld::meshCreate(UnitId unitId, const MeshRendererDesc& meshRendererDesc, const Matrix4x4& transformMatrix4x4)
{
	const MeshResource* meshResource = (const MeshResource*)resourceManager->getResourceData(RESOURCE_TYPE_MESH, meshRendererDesc.meshResource);
	const MeshGeometry* meshGeometry = meshResource->getMeshGeometry(meshRendererDesc.geometryName);
	materialManager->createMaterial(meshRendererDesc.materialResource);

	return meshManager.create(unitId, meshResource, meshGeometry, meshRendererDesc.materialResource, transformMatrix4x4);
}

void RenderWorld::meshDestroyMeshInstance(MeshInstance meshInstance)
{
	meshManager.destroy(meshInstance);
}

void RenderWorld::meshGetMeshInstanceList(UnitId unitId, Array<MeshInstance>& meshInstanceList)
{
	MeshInstance meshInstance = meshManager.getFirst(unitId);

	while (getIsValid(meshInstance))
	{
		ArrayFn::pushBack(meshInstanceList, meshInstance);
		meshInstance = meshManager.getNext(meshInstance);
	}
}

void RenderWorld::meshSetMaterial(MeshInstance meshInstance, StringId64 materialName)
{
	RIO_ASSERT(meshInstance.index < meshManager.meshInstanceData.size, "Index out of bounds");
	meshManager.meshInstanceData.materialNameList[meshInstance.index] = materialName;
}

void RenderWorld::meshSetIsVisible(MeshInstance meshInstance, bool visible)
{
	RIO_ASSERT(meshInstance.index < meshManager.meshInstanceData.size, "Index out of bounds");
}

Obb RenderWorld::meshGetObb(MeshInstance meshInstance)
{
	RIO_ASSERT(meshInstance.index < meshManager.meshInstanceData.size, "Index out of bounds");

	const Matrix4x4& worldMatrix4x4 = meshManager.meshInstanceData.worldMatrix4x4List[meshInstance.index];
	const Obb& obb = meshManager.meshInstanceData.obbList[meshInstance.index];

	Obb obbInWorldCoordinates;
	obbInWorldCoordinates.transformMatrix = obb.transformMatrix * worldMatrix4x4;
	obbInWorldCoordinates.halfExtents = obb.halfExtents;

	return obbInWorldCoordinates;
}

float RenderWorld::meshGetRayMeshIntersection(MeshInstance meshInstance, const Vector3& from, const Vector3& direction)
{
	RIO_ASSERT(meshInstance.index < meshManager.meshInstanceData.size, "Index out of bounds");
	const MeshGeometry* meshGeometry = meshManager.meshInstanceData.meshGeometryList[meshInstance.index];
	return Rio::getRayMeshIntersection(from
		, direction
		, meshManager.meshInstanceData.worldMatrix4x4List[meshInstance.index]
		, meshGeometry->vertexData.data
		, meshGeometry->vertexData.stride
		, (uint16_t*)meshGeometry->indexData.data
		, meshGeometry->indexData.indexListCount
		);
}

SpriteInstance RenderWorld::spriteCreate(UnitId unitId, const SpriteRendererDesc& spriteRendererDesc, const Matrix4x4& transformMatrix4x4)
{
	const SpriteResource* spriteResource = (const SpriteResource*)resourceManager->getResourceData(RESOURCE_TYPE_SPRITE, spriteRendererDesc.spriteResource);
	materialManager->createMaterial(spriteRendererDesc.materialResource);

	return spriteManager.create(unitId
		, spriteResource
		, spriteRendererDesc.materialResource
		, spriteRendererDesc.layer
		, spriteRendererDesc.depth
		, transformMatrix4x4
		);
}

void RenderWorld::spriteDestroySpriteInstance(UnitId unitId, SpriteInstance /*spriteInstance*/)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	spriteManager.destroy(spriteInstance);
}

SpriteInstance RenderWorld::spriteGetSpriteInstanceList(UnitId unitId)
{
	return spriteManager.getSpriteInstanceByUnitId(unitId);
}

void RenderWorld::spriteSetMaterial(UnitId unitId, StringId64 materialName)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	spriteManager.spriteInstanceData.materialNameList[spriteInstance.index] = materialName;
}

void RenderWorld::spriteSetFrame(UnitId unitId, uint32_t frameId)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	spriteManager.spriteInstanceData.frameIdList[spriteInstance.index] = frameId;
}

void RenderWorld::spriteSetIsVisible(UnitId unitId, bool isVisible)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	// TODO
	RIO_FATAL("To implement");
}

void RenderWorld::spriteSetDoFlipX(UnitId unitId, bool doFlipX)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	spriteManager.spriteInstanceData.flipXList[spriteInstance.index] = doFlipX;
}

void RenderWorld::spriteSetDoFlipY(UnitId unitId, bool doFlipY)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	spriteManager.spriteInstanceData.flipYList[spriteInstance.index] = doFlipY;
}

void RenderWorld::spriteSetLayer(UnitId unitId, uint32_t layer)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	spriteManager.spriteInstanceData.layerList[spriteInstance.index] = layer;
}

void RenderWorld::spriteSetDepth(UnitId unitId, uint32_t depth)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");
	spriteManager.spriteInstanceData.depthList[spriteInstance.index] = depth;
}

Obb RenderWorld::spriteGetObb(UnitId unitId)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");

	const Obb& obb = spriteManager.spriteInstanceData.spriteResourceList[spriteInstance.index]->obb;
	const Matrix4x4& world = spriteManager.spriteInstanceData.worldMatrix4x4List[spriteInstance.index];

	Obb o;
	o.transformMatrix = obb.transformMatrix * world;
	o.halfExtents = obb.halfExtents;

	return o;
}

float RenderWorld::spriteGetRaySpriteIntersection(UnitId unitId, const Vector3& from, const Vector3& direction, uint32_t& layer, uint32_t& depth)
{
	SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(unitId);
	RIO_ASSERT(spriteInstance.index < spriteManager.spriteInstanceData.size, "Index out of bounds");

	const SpriteManager::SpriteInstanceData& spriteInstanceData = spriteManager.spriteInstanceData;
	const float* frameData = SpriteResourceFn::getFrameData(spriteInstanceData.spriteResourceList[spriteInstance.index], spriteInstanceData.frameIdList[spriteInstance.index]);

	const float vertexList[] =
	{
		frameData[0], 0.0f, frameData[1],
		frameData[4], 0.0f, frameData[5],
		frameData[8], 0.0f, frameData[9],
		frameData[12], 0.0f, frameData[13]
	};

	const uint16_t indexList[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	layer = spriteManager.spriteInstanceData.layerList[spriteInstance.index];
	depth = spriteManager.spriteInstanceData.depthList[spriteInstance.index];

	return Rio::getRayMeshIntersection(from
		, direction
		, spriteManager.spriteInstanceData.worldMatrix4x4List[spriteInstance.index]
		, vertexList
		, sizeof(Vector3)
		, indexList
		, 6
		);
}

LightInstance RenderWorld::lightCreate(UnitId unitId, const LightDesc& lightDesc, const Matrix4x4& transformMatrix4x4)
{
	return lightManager.create(unitId, lightDesc, transformMatrix4x4);
}

void RenderWorld::lightDestroyLightInstance(UnitId unitId, LightInstance /*lightInstance*/)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	lightManager.destroy(lightInstance);
}

LightInstance RenderWorld::lightGetLightInstanceList(UnitId unitId)
{
	return lightManager.getLightInstanceByUnitId(unitId);
}

Color4 RenderWorld::lightGetColor(UnitId unitId)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	return lightManager.lightInstanceData.colorList[lightInstance.index];
}

LightType::Enum RenderWorld::lightGetLightType(UnitId unitId)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	return (LightType::Enum)lightManager.lightInstanceData.lightTypeList[lightInstance.index];
}

float RenderWorld::lightGetRange(UnitId unitId)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	return lightManager.lightInstanceData.rangeList[lightInstance.index];
}

float RenderWorld::lightIntensity(UnitId unitId)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	return lightManager.lightInstanceData.intensityList[lightInstance.index];
}

float RenderWorld::lightGetSpotAngle(UnitId unitId)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	return lightManager.lightInstanceData.spotAngleList[lightInstance.index];
}

void RenderWorld::lightSetColor(UnitId unitId, const Color4& color4)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	lightManager.lightInstanceData.colorList[lightInstance.index] = color4;
}

void RenderWorld::lightSetLightType(UnitId unitId, LightType::Enum lightType)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	lightManager.lightInstanceData.lightTypeList[lightInstance.index] = lightType;
}

void RenderWorld::lightSetRange(UnitId unitId, float range)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	lightManager.lightInstanceData.rangeList[lightInstance.index] = range;
}

void RenderWorld::lightSetIntensity(UnitId unitId, float intensity)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	lightManager.lightInstanceData.intensityList[lightInstance.index] = intensity;
}

void RenderWorld::lightSetSpotAngle(UnitId unitId, float spotAngle)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	lightManager.lightInstanceData.spotAngleList[lightInstance.index] = spotAngle;
}

void RenderWorld::lightDebugDraw(UnitId unitId, DebugLine& debugLine)
{
	LightInstance lightInstance = lightManager.getLightInstanceByUnitId(unitId);
	RIO_ASSERT(lightInstance.index < lightManager.lightInstanceData.size, "Index out of bounds");
	lightManager.debugDraw(lightInstance.index, 1, debugLine);
}

void RenderWorld::updateUnitTransformList(const UnitId* unitListBegin, const UnitId* unitListEnd, const Matrix4x4* worldMatrix4x4List)
{
	MeshManager::MeshInstanceData& meshInstanceData = meshManager.meshInstanceData;
	SpriteManager::SpriteInstanceData& spriteInstanceData = spriteManager.spriteInstanceData;
	LightManager::LightInstanceData& lightInstanceData = lightManager.lightInstanceData;

	for (; unitListBegin != unitListEnd; ++unitListBegin, ++worldMatrix4x4List)
	{
		if (meshManager.has(*unitListBegin))
		{
			MeshInstance meshInstance = meshManager.getFirst(*unitListBegin);
			meshInstanceData.worldMatrix4x4List[meshInstance.index] = *worldMatrix4x4List;
		}

		if (spriteManager.has(*unitListBegin))
		{
			SpriteInstance spriteInstance = spriteManager.getSpriteInstanceByUnitId(*unitListBegin);
			spriteInstanceData.worldMatrix4x4List[spriteInstance.index] = *worldMatrix4x4List;
		}

		if (lightManager.has(*unitListBegin))
		{
			LightInstance lightInstance = lightManager.getLightInstanceByUnitId(*unitListBegin);
			lightInstanceData.worldMatrix4x4List[lightInstance.index] = *worldMatrix4x4List;
		}
	}
}

void RenderWorld::render(const Matrix4x4& viewMatrix4x4, const Matrix4x4& projectionMatrix4x4)
{
	MeshManager::MeshInstanceData& meshInstanceData = meshManager.meshInstanceData;
	SpriteManager::SpriteInstanceData& spriteInstanceData = spriteManager.spriteInstanceData;
	LightManager::LightInstanceData& lightInstanceData = lightManager.lightInstanceData;

	for (uint32_t lightInstanceCounter = 0; lightInstanceCounter < lightInstanceData.size; ++lightInstanceCounter)
	{
		const Vector4 lightDirection = getNormalized(lightInstanceData.worldMatrix4x4List[lightInstanceCounter].z) * viewMatrix4x4;
		const Vector3 lightPosition = getTranslation(lightInstanceData.worldMatrix4x4List[lightInstanceCounter]);

		RioRenderer::setUniform(this->uniformLightPosition, getFloatPtr(lightPosition));
		RioRenderer::setUniform(this->uniformLightDirection, getFloatPtr(lightDirection));
		RioRenderer::setUniform(this->uniformLightColor, getFloatPtr(lightInstanceData.colorList[lightInstanceCounter]));
		RioRenderer::setUniform(this->uniformLightRange, &lightInstanceData.rangeList[lightInstanceCounter]);
		RioRenderer::setUniform(this->uniformLightIntensity, &lightInstanceData.intensityList[lightInstanceCounter]);

		// Render meshes
		for (uint32_t i = 0; i < meshInstanceData.firstHiddenIndex; ++i)
		{
			RioRenderer::setTransform(getFloatPtr(meshInstanceData.worldMatrix4x4List[i]));
			RioRenderer::setVertexBuffer(0, meshInstanceData.meshDataList[i].vertexBufferHandle);
			RioRenderer::setIndexBuffer(meshInstanceData.meshDataList[i].indexBufferHandle);

			materialManager->getMaterialById(meshInstanceData.materialNameList[i])->bind(*resourceManager, *shaderManager, VIEW_MESH);
		}
	}

	// Render sprites
	if (spriteInstanceData.firstHiddenIndex != 0)
	{
		RioRenderer::VertexDecl vertexDecl;
		vertexDecl.begin()
			.add(RioRenderer::Attrib::Position, 2, RioRenderer::AttribType::Float)
			.add(RioRenderer::Attrib::TexCoord0, 2, RioRenderer::AttribType::Float, false)
			.end()
			;

		RioRenderer::TransientVertexBuffer transientVertexBuffer;
		RioRenderer::allocTransientVertexBuffer(&transientVertexBuffer, 4 * spriteInstanceData.firstHiddenIndex, vertexDecl);
		RioRenderer::TransientIndexBuffer transientIndexBuffer;
		RioRenderer::allocTransientIndexBuffer(&transientIndexBuffer, 6 * spriteInstanceData.firstHiddenIndex);

		float* vertexData = (float*)transientVertexBuffer.data;
		uint16_t* indexData = (uint16_t*)transientIndexBuffer.data;

		// Render sprites
		for (uint32_t i = 0; i < spriteInstanceData.firstHiddenIndex; ++i)
		{
			const float* frameData = SpriteResourceFn::getFrameData(spriteInstanceData.spriteResourceList[i], spriteInstanceData.frameIdList[i]);

			float u0 = frameData[2]; // u
			float v0 = frameData[3]; // v

			float u1 = frameData[6]; // u
			float v1 = frameData[7]; // v

			float u2 = frameData[10]; // u
			float v2 = frameData[11]; // v

			float u3 = frameData[14]; // u
			float v3 = frameData[15]; // v

			if (spriteInstanceData.flipXList[i])
			{
				float u = 0.0f;
				u = u0; 
				u0 = u1; 
				u1 = u;
				u = u2; 
				u2 = u3; 
				u3 = u;
			}

			if (spriteInstanceData.flipYList[i])
			{
				float v = 0.0f;
				v = v0; 
				v0 = v2; 
				v2 = v;
				v = v1; 
				v1 = v3; 
				v3 = v;
			}

			vertexData[0] = frameData[0]; // x
			vertexData[1] = frameData[1]; // y
			vertexData[2] = u0;
			vertexData[3] = v0;

			vertexData[4] = frameData[4]; // x
			vertexData[5] = frameData[5]; // y
			vertexData[6] = u1;
			vertexData[7] = v1;

			vertexData[8] = frameData[8]; // x
			vertexData[9] = frameData[9]; // y
			vertexData[10] = u2;
			vertexData[11] = v2;

			vertexData[12] = frameData[12]; // x
			vertexData[13] = frameData[13]; // y
			vertexData[14] = u3;
			vertexData[15] = v3;

			vertexData += 16;

			*indexData++ = i * 4 + 0;
			*indexData++ = i * 4 + 1;
			*indexData++ = i * 4 + 2;
			*indexData++ = i * 4 + 0;
			*indexData++ = i * 4 + 2;
			*indexData++ = i * 4 + 3;

			RioRenderer::setTransform(getFloatPtr(spriteInstanceData.worldMatrix4x4List[i]));
			RioRenderer::setVertexBuffer(0, &transientVertexBuffer);
			RioRenderer::setIndexBuffer(&transientIndexBuffer, i * 6, 6);

			materialManager->getMaterialById(spriteInstanceData.materialNameList[i])->bind(*resourceManager
				, *shaderManager
				, spriteInstanceData.layerList[i] + VIEW_SPRITE_0
				, spriteInstanceData.depthList[i]
				);
		}
	}
}

void RenderWorld::debugDraw(DebugLine& debugLine)
{
	if (this->isDebugDrawingEnabled == false)
	{
		return;
	}

	MeshManager::MeshInstanceData& meshInstanceData = meshManager.meshInstanceData;

	for (uint32_t i = 0; i < meshInstanceData.size; ++i)
	{
		const Obb& obb = meshInstanceData.obbList[i];
		const Matrix4x4& worldMatrix4x4 = meshInstanceData.worldMatrix4x4List[i];
		debugLine.addObb(obb.transformMatrix * worldMatrix4x4, obb.halfExtents, COLOR4_RED);
	}

	lightManager.debugDraw(0, lightManager.lightInstanceData.size, debugLine);
}

void RenderWorld::setIsDebugDrawingEnabled(bool isDebugDrawingEnabled)
{
	this->isDebugDrawingEnabled = isDebugDrawingEnabled;
}

void RenderWorld::unitDestroyedCallback(UnitId unitId)
{
	{
		MeshInstance currentMeshInstance = meshManager.getFirst(unitId);
		MeshInstance nextMeshInstance;

		while (getIsValid(currentMeshInstance))
		{
			nextMeshInstance = meshManager.getNext(currentMeshInstance);
			meshDestroyMeshInstance(currentMeshInstance);
			currentMeshInstance = nextMeshInstance;
		}
	}

	{
		SpriteInstance firstSpriteInstance = spriteGetSpriteInstanceList(unitId);

		if (getIsValid(firstSpriteInstance))
		{
			spriteDestroySpriteInstance(unitId, firstSpriteInstance);
		}
	}

	{
		LightInstance firstLightInstance = lightGetLightInstanceList(unitId);

		if (getIsValid(firstLightInstance))
		{
			lightDestroyLightInstance(unitId, firstLightInstance);
		}
	}
}

} // namespace Rio
