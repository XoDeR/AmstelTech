#pragma once

#include "Config.h"

namespace Rio
{

struct CompileOptions;

struct DataCompiler;

struct ResourceLoader;
struct ResourceManager;

struct ResourcePackage;

struct FontResource;

struct LevelResource;

struct MaterialResource;
struct MeshResource;
struct PackageResource;
struct ShaderResource;
struct TextureResource;

struct StateMachineResource;
struct SpriteAnimationResource;
struct SpriteResource;

struct UnitResource;

#if AMSTEL_ENGINE_SOUND
struct SoundResource;
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_SCRIPT_LUA
struct LuaResource;
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_PHYSICS
struct PhysicsConfigResource;
struct PhysicsResource;
struct ActorResource;
#endif // AMSTEL_ENGINE_PHYSICS

} // namespace Rio


#define RESOURCE_TYPE_CONFIG StringId64("CONFIG")
#define RESOURCE_TYPE_FONT StringId64("FONT")
#define RESOURCE_TYPE_LEVEL StringId64("LEVEL")

#define RESOURCE_TYPE_PACKAGE StringId64("PACKAGE")

#define RESOURCE_TYPE_SHADER StringId64("SHADER")
#define RESOURCE_TYPE_TEXTURE StringId64("TEXTURE")
#define RESOURCE_TYPE_MATERIAL StringId64("MATERIAL")
#define RESOURCE_TYPE_MESH StringId64("MESH")

#define RESOURCE_TYPE_STATE_MACHINE StringId64("STATE_MACHINE")
#define RESOURCE_TYPE_SPRITE_ANIMATION StringId64("SPRITE_ANIMATION")
#define RESOURCE_TYPE_SPRITE StringId64("SPRITE")

#define RESOURCE_TYPE_UNIT StringId64("UNIT")

#if AMSTEL_ENGINE_SOUND
#define RESOURCE_TYPE_SOUND StringId64("SOUND")
#endif // AMSTEL_ENGINE_SOUND

#if AMSTEL_ENGINE_SCRIPT_LUA
#define RESOURCE_TYPE_SCRIPT StringId64("SCRIPT")
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_PHYSICS
#define RESOURCE_TYPE_PHYSICS_CONFIG StringId64("PHYSICS_CONFIG")
#define RESOURCE_TYPE_PHYSICS StringId64("PHYSICS")
#endif // AMSTEL_ENGINE_PHYSICS

#define RESOURCE_VERSION_CONFIG uint32_t(1)

#define RESOURCE_VERSION_SPRITE uint32_t(1)

#define RESOURCE_VERSION_SHADER uint32_t(1)
#define RESOURCE_VERSION_TEXTURE uint32_t(1)
#define RESOURCE_VERSION_UNIT uint32_t(1)

#define RESOURCE_VERSION_FONT uint32_t(1)
#define RESOURCE_VERSION_LEVEL uint32_t(1)
#define RESOURCE_VERSION_MATERIAL uint32_t(1)
#define RESOURCE_VERSION_MESH uint32_t(1)
#define RESOURCE_VERSION_PACKAGE uint32_t(1)

#define RESOURCE_VERSION_STATE_MACHINE uint32_t(1)
#define RESOURCE_VERSION_SPRITE_ANIMATION uint32_t(1)

#if AMSTEL_ENGINE_PHYSICS
#define RESOURCE_VERSION_PHYSICS_CONFIG uint32_t(1)
#define RESOURCE_VERSION_PHYSICS uint32_t(1)
#endif // AMSTEL_ENGINE_PHYSICS

#if AMSTEL_ENGINE_SCRIPT_LUA
#define RESOURCE_VERSION_SCRIPT uint32_t(1)
#endif // AMSTEL_ENGINE_SCRIPT_LUA

#if AMSTEL_ENGINE_SOUND
#define RESOURCE_VERSION_SOUND uint32_t(1)
#endif // AMSTEL_ENGINE_SOUND