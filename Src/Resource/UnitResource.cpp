#include "Resource/UnitResource.h"

#include "Resource/UnitCompiler.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/SortMap.h"
#include "Core/FileSystem/File.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Memory/Allocator.h"

#include "Resource/Types.h"

namespace Rio
{

namespace UnitResourceInternalFn
{
	void compile(CompileOptions& compileOptions)
	{
		UnitCompiler unitCompiler(compileOptions);
		unitCompiler.compileUnit(compileOptions.getSourcePath());

		compileOptions.write(unitCompiler.getBlob());
	}

} // namespace UnitResourceInternalFn

} // namespace Rio
