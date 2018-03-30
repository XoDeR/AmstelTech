#include "Core/Containers/Array.h"

#include "Resource/PackageResource.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResourcePackage.h"

#include "World/Types.h"

namespace Rio
{

ResourcePackage::ResourcePackage(StringId64 packageId, ResourceManager& resourceManager)
	: resourceManager(&resourceManager)
	, packageId(packageId)
{
}

ResourcePackage::~ResourcePackage()
{
	resourceManager->unload(RESOURCE_TYPE_PACKAGE, packageId);
	marker = 0;
}

void ResourcePackage::load()
{
	resourceManager->load(RESOURCE_TYPE_PACKAGE, packageId);
	resourceManager->flush();
	packageResource = (const PackageResource*)resourceManager->get(RESOURCE_TYPE_PACKAGE, packageId);

	for (uint32_t i = 0; i < ArrayFn::getCount(packageResource->resourceList); ++i)
	{
		resourceManager->load(packageResource->resourceList[i].type, packageResource->resourceList[i].name);
	}
}

void ResourcePackage::unload()
{
	for (uint32_t i = 0; i < ArrayFn::getCount(packageResource->resourceList); ++i)
	{
		resourceManager->unload(packageResource->resourceList[i].type, packageResource->resourceList[i].name);
	}
}

void ResourcePackage::flush()
{
	resourceManager->flush();
}

bool ResourcePackage::hasLoaded() const
{
	for (uint32_t i = 0; i < ArrayFn::getCount(packageResource->resourceList); ++i)
	{
		if (!resourceManager->can_get(packageResource->resourceList[i].type, packageResource->resourceList[i].name))
		{
			return false;
		}
	}

	return true;
}

} // namespace Rio
