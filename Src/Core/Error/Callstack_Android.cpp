#include "Core/Platform.h"

#if RIO_PLATFORM_ANDROID

#include "Core/Strings/StringStream.h"

namespace Rio
{

namespace ErrorFn
{
	void callstack(StringStream& stringStream)
	{
		stringStream << "Not supported";
	}

} // namespace ErrorFn

} // namespace Rio

#endif // RIO_PLATFORM_ANDROID
