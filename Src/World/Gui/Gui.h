#pragma once

#include "Core/Math/Types.h"
#include "Core/Math/Matrix4x4.h"

#include "Resource/Types.h"

#include "World/Types.h"

#include "RioRenderer/RioRenderer.h"

#include "World/Renderer/ShaderManager.h"
#include "World/Renderer/Material.h"

#include <stdio.h>

namespace Rio
{

struct GuiBuffer
{
	ShaderManager* shaderManager = nullptr;

	uint32_t vertexListCount = 0;
	uint32_t indexListCount = 0;

	RioRenderer::VertexDecl positionTextureCoordColorVertexDecl;
	RioRenderer::TransientVertexBuffer transientVertexBuffer;
	RioRenderer::TransientIndexBuffer transientIndexBuffer;

	GuiBuffer(ShaderManager& shaderManager)
		: shaderManager(&shaderManager)
	{
	}

	void* getVertexBufferEnd()
	{
		return transientVertexBuffer.data + this->vertexListCount * 24;
	}

	void* getIndexBufferEnd()
	{
		return transientIndexBuffer.data + this->indexListCount * 2;
	}

	void create()
	{
		this->positionTextureCoordColorVertexDecl.begin()
			.add(RioRenderer::Attrib::Position, 3, RioRenderer::AttribType::Float)
			.add(RioRenderer::Attrib::TexCoord0, 2, RioRenderer::AttribType::Float, true)
			.add(RioRenderer::Attrib::Color0, 4, RioRenderer::AttribType::Uint8, true)
			.end()
			;
	}

	void reset()
	{
		this->vertexListCount = 0;
		this->indexListCount = 0;

		RioRenderer::allocTransientVertexBuffer(&transientVertexBuffer, 4096, this->positionTextureCoordColorVertexDecl);
		RioRenderer::allocTransientIndexBuffer(&transientIndexBuffer, 6144);
	}

	void submit(uint32_t vertexListCount, uint32_t indexListCount, const Matrix4x4& worldMatrix4x4)
	{
		RioRenderer::setVertexBuffer(0, &transientVertexBuffer, this->vertexListCount, vertexListCount);
		RioRenderer::setIndexBuffer(&transientIndexBuffer, this->indexListCount, indexListCount);
		RioRenderer::setTransform(getFloatPtr(worldMatrix4x4));

		shaderManager->submit(StringId32("gui"), 2);

		this->vertexListCount += vertexListCount;
		this->indexListCount += indexListCount;
	}

	void submitWithMaterial(uint32_t vertexListCount, uint32_t indexListCount, const Matrix4x4& worldMatrix4x4, ResourceManager& resourceManager, Material* material)
	{
		RioRenderer::setVertexBuffer(0, &transientVertexBuffer, this->vertexListCount, vertexListCount);
		RioRenderer::setIndexBuffer(&transientIndexBuffer, this->indexListCount, indexListCount);
		RioRenderer::setTransform(getFloatPtr(worldMatrix4x4));

		material->bind(resourceManager, *shaderManager, 2);

		this->vertexListCount += vertexListCount;
		this->indexListCount += indexListCount;
	}
};

// Immediate mode GUI
struct Gui
{
	struct VertexData
	{
		Vector3 position;
		Vector2 uv;
		uint32_t colorPacked = UINT32_MAX; // white
	};

	struct IndexData
	{
		uint16_t a = 0;
		uint16_t b = 0;
	};

	uint32_t marker = DEBUG_GUI_MARKER;
	GuiBuffer* guiBuffer = nullptr;

	ResourceManager* resourceManager = nullptr;
	ShaderManager* shaderManager = nullptr;
	MaterialManager* materialManager = nullptr;

	Matrix4x4 worldMatrix = MATRIX4X4_IDENTITY;

	Gui(GuiBuffer& GuiBuffer, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager);
	~Gui();

	void move(const Vector2& position);

	void drawTriangle3d(const Vector3& a, const Vector3& b, const Vector3& c, const Color4& color);
	void drawTriangle(const Vector2& a, const Vector2& b, const Vector2& c, const Color4& color);
	void drawRect3D(const Vector3& position, const Vector2& size, const Color4& color);
	void drawRect(const Vector2& position, const Vector2& size, const Color4& color);
	void drawImage3D(const Vector3& position, const Vector2& size, StringId64 material, const Color4& color);
	void drawImage(const Vector2& position, const Vector2& size, StringId64 material, const Color4& color);
	void drawImage3DUv(const Vector3& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);
	void drawImageUv(const Vector2& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color);
	void drawText3D(const Vector3& position, uint32_t fontSize, const char* str, StringId64 font, StringId64 material, const Color4& color);
	void drawText(const Vector2& position, uint32_t fontSize, const char* str, StringId64 font, StringId64 material, const Color4& color);
};

} // namespace Rio
