#pragma once

#include "Core/Containers/Types.h"
#include "Core/Math/Types.h"
#include "Core/Strings/StringId.h"

#include "Resource/Types.h"

#include "World/Types.h"

#include "World/Renderer/LightManager.h"
#include "World/Renderer/MeshManager.h"
#include "World/Renderer/SpriteManager.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

struct RenderWorld
{
	uint32_t marker = RENDER_WORLD_MARKER;
	Allocator* allocator = nullptr;
	ResourceManager* resourceManager = nullptr;
	ShaderManager* shaderManager = nullptr;
	MaterialManager* materialManager = nullptr;
	UnitManager* unitManager = nullptr;

	RioRenderer::UniformHandle uniformLightPosition;
	RioRenderer::UniformHandle uniformLightDirection;
	RioRenderer::UniformHandle uniformLightColor;
	RioRenderer::UniformHandle uniformLightRange;
	RioRenderer::UniformHandle uniformLightIntensity;

	bool isDebugDrawingEnabled = false;

	MeshManager meshManager;
	SpriteManager spriteManager;
	LightManager lightManager;

	RenderWorld(Allocator& a, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager, UnitManager& unitManager);
	~RenderWorld();

	// Creates a new mesh instance
	MeshInstance meshCreate(UnitId unitId, const MeshRendererDesc& meshRendererDesc, const Matrix4x4& transformMatrix4x4);

	// Destroys the mesh <meshInstance>
	void meshDestroyMeshInstance(MeshInstance meshInstance);

	// Returns the mesh instances of the unit <unitId>
	void meshGetMeshInstanceList(UnitId unitId, Array<MeshInstance>& meshInstanceList);

	// Sets the material <materialName> of the mesh <meshInstance>
	void meshSetMaterial(MeshInstance meshInstance, StringId64 materialName);

	// Sets whether the mesh <meshInstance> is <isVisible>
	void meshSetIsVisible(MeshInstance meshInstance, bool isVisible);

	// Returns the Obb of the mesh <meshInstance>
	Obb meshGetObb(MeshInstance meshInstance);

	// Returns the distance along ray (<from>, <direction>) to intersection point with mesh <meshInstance> or -1.0 if no intersection
	float meshGetRayMeshIntersection(MeshInstance meshInstance, const Vector3& from, const Vector3& direction);

	// Creates a new sprite instance
	SpriteInstance spriteCreate(UnitId unitId, const SpriteRendererDesc& spriteRendererDesc, const Matrix4x4& transformMatrix4x4);

	// Destroys the sprite of the <unitId>
	void spriteDestroySpriteInstance(UnitId unitId, SpriteInstance spriteInstance);

	// Returns the sprite instances of the <unitId>
	SpriteInstance spriteGetSpriteInstanceList(UnitId unitId);

	// Sets the material <materialName> of the sprite
	void spriteSetMaterial(UnitId unitId, StringId64 materialName);

	// Sets the frame <frameId> of the sprite
	void spriteSetFrame(UnitId unitId, uint32_t frameId);

	// Sets whether the sprite is <isVisible>
	void spriteSetIsVisible(UnitId unitId, bool isVisible);

	// Sets whether to flip the sprite on the x-axis
	void spriteSetDoFlipX(UnitId unitId, bool doFlipX);

	// Sets whether to flip the sprite on the y-axis
	void spriteSetDoFlipY(UnitId unitId, bool doFlipY);

	// Sets the layer of the sprite
	void spriteSetLayer(UnitId unitId, uint32_t layer);

	// Sets the depth of the sprite
	void spriteSetDepth(UnitId unitId, uint32_t depth);

	// Returns the Obb of the sprite
	Obb spriteGetObb(UnitId unitId);

	// Returns the distance along ray (<from>, <direction>) to intersection point with sprite or -1.0 if no intersection
	float spriteGetRaySpriteIntersection(UnitId unitId, const Vector3& from, const Vector3& direction, uint32_t& layer, uint32_t& depth);

	// Creates a new light instance
	LightInstance lightCreate(UnitId unitId, const LightDesc& lightDesc, const Matrix4x4& transformMatrix4x4);

	// Destroys the light
	void lightDestroyLightInstance(UnitId unitId, LightInstance lightInstance);

	// Returns the light of the <unitId>
	LightInstance lightGetLightInstanceList(UnitId unitId);

	// Returns the type of the light
	LightType::Enum lightGetLightType(UnitId unitId);

	// Returns the color of the light
	Color4 lightGetColor(UnitId unitId);

	// Returns the range of the light
	float lightGetRange(UnitId unitId);

	// Returns the intensity of the light
	float lightIntensity(UnitId unitId);

	// Returns the spot angle of the light
	float lightGetSpotAngle(UnitId unitId);

	// Sets the <lightType> of the light
	void lightSetLightType(UnitId unitId, LightType::Enum lightType);

	// Sets the <color> of the light
	void lightSetColor(UnitId unitId, const Color4& color4);

	// Sets the <range> of the light
	void lightSetRange(UnitId unitId, float range);

	// Sets the <intensity> of the light
	void lightSetIntensity(UnitId unitId, float intensity);

	// Sets the <spotAngle> of the light
	void lightSetSpotAngle(UnitId unitId, float spotAngle);

	// Fills <debugLine> with debug lines from the light
	void lightDebugDraw(UnitId unitId, DebugLine& debugLine);

	void updateUnitTransformList(const UnitId* unitIdListBegin, const UnitId* unitIdListEnd, const Matrix4x4* worldMatrix4x4List);

	void render(const Matrix4x4& viewMatrix4x4, const Matrix4x4& projectionMatrix4x4);

	// Sets whether to <isDebugDrawingEnabled> debug drawing
	void setIsDebugDrawingEnabled(bool isDebugDrawingEnabled);

	// Fills <debugLine> with debug lines
	void debugDraw(DebugLine& debugLine);

	void unitDestroyedCallback(UnitId unitId);
};

} // namespace Rio
