#include "World/Gui/Gui.h"

#include "Core/Math/Color4.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Strings/String.h"
#include "Core/Strings/Utf8.h"

#include "Resource/Gui/FontResource.h"
#include "Resource/Renderer/MaterialResource.h"
#include "Resource/ResourceManager.h"

#include "World/Renderer/MaterialManager.h"
#include "World/Renderer/ShaderManager.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

Gui::Gui(GuiBuffer& guiBuffer, ResourceManager& resourceManager, ShaderManager& shaderManager, MaterialManager& materialManager)
	: guiBuffer(&guiBuffer)
	, resourceManager(&resourceManager)
	, shaderManager(&shaderManager)
	, materialManager(&materialManager)
{
}

Gui::~Gui()
{
	marker = 0;
}

void Gui::move(const Vector2& position)
{
	setTranslation(worldMatrix, createVector3(position.x, position.y, 0));
}

void Gui::drawTriangle3d(const Vector3& a, const Vector3& b, const Vector3& c, const Color4& color)
{
	VertexData* vertexDataList = (VertexData*)this->guiBuffer->getVertexBufferEnd();
	vertexDataList[0].position.x = a.x;
	vertexDataList[0].position.y = a.y;
	vertexDataList[0].position.z = a.z;
	vertexDataList[0].uv.x = 0.0f;
	vertexDataList[0].uv.y = 0.0f;
	vertexDataList[0].colorPacked = getPackedAbgr(color);

	vertexDataList[1].position.x = b.x;
	vertexDataList[1].position.y = b.y;
	vertexDataList[1].position.z = b.z;
	vertexDataList[1].uv.x = 1.0f;
	vertexDataList[1].uv.y = 0.0f;
	vertexDataList[1].colorPacked = getPackedAbgr(color);

	vertexDataList[2].position.x = c.x;
	vertexDataList[2].position.y = c.y;
	vertexDataList[2].position.z = c.z;
	vertexDataList[2].uv.x = 1.0f;
	vertexDataList[2].uv.y = 1.0f;
	vertexDataList[2].colorPacked = getPackedAbgr(color);

	uint16_t* indexList = (uint16_t*)this->guiBuffer->getIndexBufferEnd();
	indexList[0] = 0;
	indexList[1] = 1;
	indexList[2] = 2;

	this->guiBuffer->submit(3, 3, worldMatrix);
}

void Gui::drawTriangle(const Vector2& a, const Vector2& b, const Vector2& c, const Color4& color)
{
	drawTriangle3d(createVector3(a.x, a.y, 0.0f), createVector3(b.x, b.y, 0.0f), createVector3(c.x, c.y, 0.0f), color);
}

void Gui::drawRect3D(const Vector3& position, const Vector2& size, const Color4& color)
{
	VertexData* vertexDataList = (VertexData*)this->guiBuffer->getVertexBufferEnd();
	vertexDataList[0].position.x = position.x;
	vertexDataList[0].position.y = position.y;
	vertexDataList[0].position.z = position.z;
	vertexDataList[0].uv.x = 0.0f;
	vertexDataList[0].uv.y = 1.0f;
	vertexDataList[0].colorPacked = getPackedAbgr(color);

	vertexDataList[1].position.x = position.x + size.x;
	vertexDataList[1].position.y = position.y;
	vertexDataList[1].position.z = position.z;
	vertexDataList[1].uv.x = 1.0f;
	vertexDataList[1].uv.y = 1.0f;
	vertexDataList[1].colorPacked = getPackedAbgr(color);

	vertexDataList[2].position.x = position.x + size.x;
	vertexDataList[2].position.y = position.y + size.y;
	vertexDataList[2].position.z = position.z;
	vertexDataList[2].uv.x = 1.0f;
	vertexDataList[2].uv.y = 0.0f;
	vertexDataList[2].colorPacked = getPackedAbgr(color);

	vertexDataList[3].position.x = position.x;
	vertexDataList[3].position.y = position.y + size.y;
	vertexDataList[3].position.z = position.z;
	vertexDataList[3].uv.x = 0.0f;
	vertexDataList[3].uv.y = 0.0f;
	vertexDataList[3].colorPacked = getPackedAbgr(color);

	uint16_t* indexList = (uint16_t*)this->guiBuffer->getIndexBufferEnd();
	indexList[0] = 0;
	indexList[1] = 1;
	indexList[2] = 2;
	indexList[3] = 0;
	indexList[4] = 2;
	indexList[5] = 3;

	this->guiBuffer->submit(4, 6, worldMatrix);
}

void Gui::drawRect(const Vector2& position, const Vector2& size, const Color4& color)
{
	drawRect3D(createVector3(position.x, position.y, 0.0f), size, color);
}

void Gui::drawImage3DUv(const Vector3& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color)
{
	VertexData* vertexDataList = (VertexData*)this->guiBuffer->getVertexBufferEnd();
	vertexDataList[0].position.x = position.x;
	vertexDataList[0].position.y = position.y;
	vertexDataList[0].position.z = position.z;
	vertexDataList[0].uv.x = 0.0f;
	vertexDataList[0].uv.y = 1.0f;
	vertexDataList[0].colorPacked = getPackedAbgr(color);

	vertexDataList[1].position.x = position.x + size.x;
	vertexDataList[1].position.y = position.y;
	vertexDataList[1].position.z = position.z;
	vertexDataList[1].uv.x = 1.0f;
	vertexDataList[1].uv.y = 1.0f;
	vertexDataList[1].colorPacked = getPackedAbgr(color);

	vertexDataList[2].position.x = position.x + size.x;
	vertexDataList[2].position.y = position.y + size.y;
	vertexDataList[2].position.z = position.z;
	vertexDataList[2].uv.x = 1.0f;
	vertexDataList[2].uv.y = 0.0f;
	vertexDataList[2].colorPacked = getPackedAbgr(color);

	vertexDataList[3].position.x = position.x;
	vertexDataList[3].position.y = position.y + size.y;
	vertexDataList[3].position.z = position.z;
	vertexDataList[3].uv.x = 0.0f;
	vertexDataList[3].uv.y = 0.0f;
	vertexDataList[3].colorPacked = getPackedAbgr(color);

	uint16_t* indexList = (uint16_t*)this->guiBuffer->getIndexBufferEnd();
	indexList[0] = 0;
	indexList[1] = 1;
	indexList[2] = 2;
	indexList[3] = 0;
	indexList[4] = 2;
	indexList[5] = 3;

	materialManager->createMaterial(material);
	this->guiBuffer->submitWithMaterial(4
		, 6
		, worldMatrix
		, *resourceManager
		, materialManager->getMaterialById(material)
		);
}

void Gui::drawImageUv(const Vector2& position, const Vector2& size, const Vector2& uv0, const Vector2& uv1, StringId64 material, const Color4& color)
{
	drawImage3DUv(createVector3(position.x, position.y, 0.0f), size, uv0, uv1, material, color);
}

void Gui::drawImage3D(const Vector3& position, const Vector2& size, StringId64 material, const Color4& color)
{
	drawImage3DUv(position, size, VECTOR2_ZERO, VECTOR2_ONE, material, color);
}

void Gui::drawImage(const Vector2& position, const Vector2& size, StringId64 material, const Color4& color)
{
	drawImage3D(createVector3(position.x, position.y, 0.0f), size, material, color);
}

void Gui::drawText3D(const Vector3& position, uint32_t fontSize, const char* textString, StringId64 font, StringId64 material, const Color4& color)
{
	materialManager->createMaterial(material);

	const FontResource* fontResource = (FontResource*)resourceManager->getResourceData(RESOURCE_TYPE_FONT, font);
	const float scale = (float)fontSize / (float)fontResource->fontSize;

	float penX = 0.0f;
	float penY = 0.0f;
	float penAdvanceX = 0.0f;
	float penAdvanceY = 0.0f;

	VertexData* vertexDataList = (VertexData*)this->guiBuffer->getVertexBufferEnd();
	uint16_t* indexBuffer = (uint16_t*)this->guiBuffer->getIndexBufferEnd();

	const uint32_t length = getStrLen32(textString);
	uint32_t vertexListCount = 0;
	uint32_t indexListCount = 0;
	for (uint32_t i = 0; i < length; ++i)
	{
		switch (textString[i])
		{
		case '\n':
			penAdvanceX = 0.0f;
			penAdvanceY -= scale * fontResource->fontSize;
			continue;

		case '\t':
			penAdvanceX += scale * fontSize * 4;
			continue;
		}

		uint32_t state = 0;
		uint32_t codePoint = 0;

		if (Utf8Fn::decode(&state, &codePoint, textString[i]) == UTF8_ACCEPT)
		{
			const GlyphData* glyph = FontResourceFn::getGlyphData(fontResource, codePoint);

			const float baseline = glyph->height - glyph->yOffset;

			// Set pen position
			penX = position.x + scale * glyph->xOffset;
			penY = position.y - scale * baseline;

			// Position coords
			const float x0 = (penX + penAdvanceX);
			const float y0 = (penY + penAdvanceY);
			const float x1 = (penX + penAdvanceX + scale * glyph->width );
			const float y1 = (penY + penAdvanceY + scale * glyph->height);

			// Texture coords
			const float u0 = glyph->x / fontResource->textureSize;
			const float v1 = glyph->y / fontResource->textureSize; // Upper-left character corner
			const float u1 = glyph->width / fontResource->textureSize + u0;
			const float v0 = glyph->height / fontResource->textureSize + v1; // Bottom-left character corner

			// Fill vertex buffer
			vertexDataList[0].position.x = x0;
			vertexDataList[0].position.y = y0;
			vertexDataList[0].position.z = position.z;
			vertexDataList[0].uv.x = u0;
			vertexDataList[0].uv.y = v0;
			vertexDataList[0].colorPacked = getPackedAbgr(color);

			vertexDataList[1].position.x = x1;
			vertexDataList[1].position.y = y0;
			vertexDataList[1].position.z = position.z;
			vertexDataList[1].uv.x = u1;
			vertexDataList[1].uv.y = v0;
			vertexDataList[1].colorPacked = getPackedAbgr(color);

			vertexDataList[2].position.x = x1;
			vertexDataList[2].position.y = y1;
			vertexDataList[2].position.z = position.z;
			vertexDataList[2].uv.x = u1;
			vertexDataList[2].uv.y = v1;
			vertexDataList[2].colorPacked = getPackedAbgr(color);

			vertexDataList[3].position.x = x0;
			vertexDataList[3].position.y = y1;
			vertexDataList[3].position.z = position.z;
			vertexDataList[3].uv.x = u0;
			vertexDataList[3].uv.y = v1;
			vertexDataList[3].colorPacked = getPackedAbgr(color);

			// Fill index buffer
			indexBuffer[0] = vertexListCount + 0;
			indexBuffer[1] = vertexListCount + 1;
			indexBuffer[2] = vertexListCount + 2;
			indexBuffer[3] = vertexListCount + 0;
			indexBuffer[4] = vertexListCount + 2;
			indexBuffer[5] = vertexListCount + 3;

			// Advance pen position
			penAdvanceX += scale * glyph->xAdvance;

			vertexDataList += 4;
			indexBuffer += 6;
			vertexListCount += 4;
			indexListCount += 6;
		}
	}

	this->guiBuffer->submitWithMaterial(vertexListCount
		, indexListCount
		, worldMatrix
		, *resourceManager
		, materialManager->getMaterialById(material)
		);
}

void Gui::drawText(const Vector2& position, uint32_t fontSize, const char* textString, StringId64 font, StringId64 material, const Color4& color)
{
	drawText3D(createVector3(position.x, position.y, 0.0f), fontSize, textString, font, material, color);
}

} // namespace Rio
