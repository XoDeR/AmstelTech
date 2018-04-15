#pragma once

#include "Core/Math/Types.h"
#include "Resource/Types.h"
#include "World/Types.h"

namespace Rio
{

struct Material
{
	const MaterialResource* materialResource = nullptr;
	char* materialData = nullptr;

	void bind(ResourceManager& resourceManager, ShaderManager& shaderManager, uint8_t view, int32_t depth = 0) const;

	// Sets the <value> of the variable <name>
	void setFloat(StringId32 name, float value);

	// Sets the <value> of the variable <name>
	void setVector2(StringId32 name, const Vector2& value);

	// Sets the <value> of the variable <name>
	void setVector3(StringId32 name, const Vector3& value);
};

} // namespace Rio
