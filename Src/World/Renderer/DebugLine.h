#pragma once

#include "Core/Math/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

#include "Resource/Types.h"

#include "World/Types.h"

#include "RioRenderer/RioRenderer.h"

namespace Rio
{

// Draws lines
struct DebugLine
{
	// Default number of segments
	static const uint32_t CIRCLE_SEGMENT_LIST_COUNT_DEFAULT = 36;
	static const uint32_t LINE_LIST_COUNT_MAX = 32768;

	struct Line
	{
		Vector3 p0;
		uint32_t c0 = 0;
		Vector3 p1;
		uint32_t c1 = 0;
	};

	uint32_t marker = DEBUG_LINE_MARKER;
	ShaderManager* shaderManager = nullptr;
	StringId32 shaderString;
	RioRenderer::VertexDecl vertexDecl;

	uint32_t lineListCount = 0;
	Line lineList[LINE_LIST_COUNT_MAX];

	// Whether to enable <enableDepthTest>
	DebugLine(ShaderManager& shaderManager, bool enableDepthTest);
	~DebugLine();

	// Adds a line from <start> to <end> with the given <color>
	void addLine(const Vector3& start, const Vector3& end, const Color4& color);

	// Adds lines for each axis with the given <length>
	void addAxisList(const Matrix4x4& matrix4x4, float length = 1.0f);

	// Adds an arc at <center> with the given <radius> and <planeNormal> and <midPointNormal> vectors
	void addArc(const Vector3& center, float radius, const Vector3& planeNormal, const Vector3& midPointNormal, const Color4& color, uint32_t circleSegmentListCount = CIRCLE_SEGMENT_LIST_COUNT_DEFAULT);

	// Adds a circle at <center> with the given <radius> and <normal> vector
	void addCircle(const Vector3& center, float radius, const Vector3& normal, const Color4& color, uint32_t circleSegmentListCount = CIRCLE_SEGMENT_LIST_COUNT_DEFAULT);

	// Adds a cone with the base centered at <baseCenter> and the tip at <tip>
	void addCone(const Vector3& baseCenter, const Vector3& tip, float radius, const Color4& color, uint32_t circleSegmentListCount = CIRCLE_SEGMENT_LIST_COUNT_DEFAULT);

	// Adds a sphere at <center> with the given <radius> and <color>
	void addSphere(const Vector3& center, const float radius, const Color4& color, uint32_t circleSegmentListCount = CIRCLE_SEGMENT_LIST_COUNT_DEFAULT);

	// Adds an oriented bounding box
	// <transformMatrix> describes the position and orientation of the box
	// <halfExtents> describes the size of the box along the axis
	void addObb(const Matrix4x4& transformMatrix, const Vector3& halfExtents, const Color4& color);

	// Adds a frustum defined by <modelViewProjectionMatrix4x4>
	void addFrustum(const Matrix4x4& modelViewProjectionMatrix4x4, const Color4& color);

	// Adds the mesh described by {<vertexList>, <stride>, <indexList>, <count>}
	void addMesh(const Matrix4x4& transformMatrix, const void* vertexList, uint32_t stride, const uint16_t* indexList, uint32_t count, const Color4& color);

	// Adds the meshes or sprites' OBBs from the unit <name>
	void addUnit(ResourceManager& resourceManager, const Matrix4x4& transformMatrix, StringId64 name, const Color4& color);

	// Resets all the lines
	void reset();

	// Submits the lines to renderer for drawing
	void submit();
};

} // namespace Rio
