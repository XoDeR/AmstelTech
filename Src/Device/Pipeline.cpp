#include "Device/Pipeline.h"

#include "Core/Types.h"

#if AMSTEL_ENGINE_WORLD
#include "World/ShaderManager.h"
#endif // AMSTEL_ENGINE_WORLD

#include "RioCore/Math.h"

namespace Rio
{

struct PosTexCoord0Vertex
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float u = 0.0f;
	float v = 0.0f;

	static void init()
	{
		vertexDeclStatic
			.begin()
			.add(RioRenderer::Attrib::Position, 3, RioRenderer::AttribType::Float)
			.add(RioRenderer::Attrib::TexCoord0, 2, RioRenderer::AttribType::Float)
			.end();
	}

	static RioRenderer::VertexDecl vertexDeclStatic;
};

RioRenderer::VertexDecl PosTexCoord0Vertex::vertexDeclStatic;

void screenSpaceQuad(float textureWidth, float textureHeight, float texelHalf, bool originBottomLeft, float width = 1.0f, float height = 1.0f)
{
	if (3 == RioRenderer::getAvailableTransientVertexBuffer(3, PosTexCoord0Vertex::vertexDeclStatic))
	{
		RioRenderer::TransientVertexBuffer transientVertexBuffer;
		RioRenderer::allocTransientVertexBuffer(&transientVertexBuffer, 3, PosTexCoord0Vertex::vertexDeclStatic);
		PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)transientVertexBuffer.data;

		const float minX = -width;
		const float maxX =  width;
		const float minY = 0.0f;
		const float maxY = height * 2.0f;

		const float texelHalfW = texelHalf/textureWidth;
		const float texelHalfH = texelHalf/textureHeight;
		const float minU = -1.0f + texelHalfW;
		const float maxU =  1.0f + texelHalfH;

		const float zLocal = 0.0f;

		float minV = texelHalfH;
		float maxV = 2.0f + texelHalfH;

		if (originBottomLeft == true)
		{
			float temp = minV;
			minV = maxV;
			maxV = temp;

			minV -= 1.0f;
			maxV -= 1.0f;
		}

		vertex[0].x = minX;
		vertex[0].y = minY;
		vertex[0].z = zLocal;
		vertex[0].u = minU;
		vertex[0].v = minV;

		vertex[1].x = maxX;
		vertex[1].y = minY;
		vertex[1].z = zLocal;
		vertex[1].u = maxU;
		vertex[1].v = minV;

		vertex[2].x = maxX;
		vertex[2].y = maxY;
		vertex[2].z = zLocal;
		vertex[2].u = maxU;
		vertex[2].v = maxV;

		RioRenderer::setVertexBuffer(0, &transientVertexBuffer);
	}
}

Pipeline::Pipeline()
{
}

void Pipeline::create(uint16_t width, uint16_t height)
{
	PosTexCoord0Vertex::init();

	textureColor = RioRenderer::createUniform("s_texColor",  RioRenderer::UniformType::Int1);

	reset(width, height);
}

void Pipeline::destroy()
{
	RioRenderer::destroy(frameBufferHandle);
	RioRenderer::destroy(textureHandleBufferList[1]);
	RioRenderer::destroy(textureHandleBufferList[0]);
	RioRenderer::destroy(textureColor);
}

void Pipeline::reset(uint16_t width, uint16_t height)
{
	for (uint32_t i = 0; i < countof(textureHandleBufferList); ++i)
	{
		if (RioRenderer::isValid(textureHandleBufferList[i]))
		{
			RioRenderer::destroy(textureHandleBufferList[i]);
		}
	}

	textureHandleBufferList[0] = RioRenderer::createTexture2D(width
		, height
		, false
		, 1
		, RioRenderer::TextureFormat::BGRA8
		, RIO_RENDERER_TEXTURE_RT
		);

	textureHandleBufferList[1] = RioRenderer::createTexture2D(width
		, height
		, false
		, 1
		, RioRenderer::TextureFormat::D24S8
		, RIO_RENDERER_TEXTURE_RT
		);

	if (RioRenderer::isValid(frameBufferHandle))
	{
		RioRenderer::destroy(frameBufferHandle);
	}

	frameBufferHandle = RioRenderer::createFrameBuffer(countof(textureHandleBufferList), textureHandleBufferList);
}

#if AMSTEL_ENGINE_WORLD
void Pipeline::render(ShaderManager& shaderManager, StringId32 program, uint8_t view, uint16_t width, uint16_t height)
{
	const RioRenderer::Caps* rendererCapabilities = RioRenderer::getCaps();

	float orthoMatrix[16];
	RioCore::getMatrixOrtho(orthoMatrix, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, rendererCapabilities->homogeneousDepth);

	RioRenderer::setViewClear(view
		, RIO_RENDERER_CLEAR_COLOR | RIO_RENDERER_CLEAR_DEPTH | RIO_RENDERER_CLEAR_STENCIL
		, 0x000000ff
		, 1.0f
		, 0
		);

	RioRenderer::setViewFrameBuffer(view, RIO_RENDERER_INVALID_HANDLE_UINT16_T);
	RioRenderer::setViewRect(view, 0, 0, width, height);
	RioRenderer::setViewTransform(view, nullptr, orthoMatrix);

	const uint32_t samplerFlags = 0
		| RIO_RENDERER_TEXTURE_RT
		| RIO_RENDERER_TEXTURE_MIN_POINT
		| RIO_RENDERER_TEXTURE_MAG_POINT
		| RIO_RENDERER_TEXTURE_MIP_POINT
		| RIO_RENDERER_TEXTURE_U_CLAMP
		| RIO_RENDERER_TEXTURE_V_CLAMP
		;

	RioRenderer::setTexture(0, textureColor, textureHandleBufferList[0], samplerFlags);
	screenSpaceQuad(width, height, 0.0f, rendererCapabilities->originBottomLeft);

	shaderManager.submit(program, view, 0, RIO_RENDERER_STATE_RGB_WRITE | RIO_RENDERER_STATE_ALPHA_WRITE);
}
#endif // AMSTEL_ENGINE_WORLD

} // namespace Rio
