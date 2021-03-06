#pragma once

#include "Config.h"

#include "Core/Strings/StringId.h"

#if AMSTEL_ENGINE_WORLD
#include "World/Types.h"
#endif // AMSTEL_ENGINE_WORLD

#include "RioRenderer/RioRenderer.h"

#define VIEW_SPRITE_0 1
#define VIEW_SPRITE_1 2
#define VIEW_SPRITE_2 3
#define VIEW_SPRITE_3 4
#define VIEW_SPRITE_4 5
#define VIEW_SPRITE_5 6
#define VIEW_SPRITE_6 7
#define VIEW_SPRITE_7 8
#define VIEW_MESH 16
#define VIEW_DEBUG 17
#define VIEW_GUI 128
#define VIEW_IMGUI 129

namespace Rio
{

struct ShaderManager;

struct Pipeline
{
	RioRenderer::TextureHandle textureHandleBufferList[2] = { RIO_RENDERER_INVALID_HANDLE_UINT16_T, RIO_RENDERER_INVALID_HANDLE_UINT16_T };
	RioRenderer::FrameBufferHandle frameBufferHandle = RIO_RENDERER_INVALID_HANDLE_UINT16_T;
	RioRenderer::UniformHandle textureColor;

	Pipeline();

	void create(uint16_t width, uint16_t height);
	void destroy();
	void reset(uint16_t width, uint16_t height);

	void render(ShaderManager& shaderManager, StringId32 program, uint8_t viewId, uint16_t width, uint16_t height);
};

} // namespace Rio
