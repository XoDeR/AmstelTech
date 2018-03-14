#include "Core/Guid.h"

#include "Core/Platform.h"
#include "Core/Strings/DynamicString.h"

#if RIO_PLATFORM_POSIX
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#elif RIO_PLATFORM_WINDOWS
	#include <objbase.h>
#endif // RIO_PLATFORM_POSIX

namespace Rio
{

namespace GuidFn
{
	Guid getNewGuid()
	{
		Guid guid;
#if RIO_PLATFORM_POSIX
		int fd = open("/dev/urandom", O_RDONLY);
		RIO_ASSERT(fd != -1, "open: errno = %d", errno);
		ssize_t rb = read(fd, &guid, sizeof(guid));
		RIO_ENSURE(rb == sizeof(guid));
		RIO_UNUSED(rb);
		close(fd);
		guid.data3 = (guid.data3 & 0x4fffu) | 0x4000u;
		guid.data4 = (guid.data4 & 0x3fffffffffffffffu) | 0x8000000000000000u;
#elif RIO_PLATFORM_WINDOWS
		HRESULT hr = CoCreateGuid((GUID*)&guid);
		RIO_ASSERT(hr == S_OK, "CoCreateGuid: error");
		RIO_UNUSED(hr);
#endif // RIO_PLATFORM_POSIX
		return guid;
	}

	Guid parseFromString(const char* str)
	{
		Guid guid;
		tryParseFromString(guid, str);
		return guid;
	}

	bool tryParseFromString(Guid& guid, const char* str)
	{
		RIO_ENSURE(nullptr != str);
		uint32_t a = 0;
		uint32_t b = 0;
		uint32_t c = 0;
		uint32_t d = 0;
		uint32_t e = 0;
		uint32_t f = 0;
		int num = sscanf(str, "%8x-%4x-%4x-%4x-%4x%8x", &a, &b, &c, &d, &e, &f);
		guid.data1 = a;
		guid.data2 = (uint16_t)(b & 0x0000ffffu);
		guid.data3 = (uint16_t)(c & 0x0000ffffu);
		guid.data4 = (uint64_t)(d & 0x0000ffffu) << 48 | (uint64_t)(e & 0x0000ffffu) << 32 | (uint64_t)f;
		return num == 6;
	}

	void toString(const Guid& guid, DynamicString& str)
	{
		char buf[36+1];
		Rio::snPrintF(buf, sizeof(buf), "%.8x-%.4x-%.4x-%.4x-%.4x%.8x"
			, guid.data1
			, guid.data2
			, guid.data3
			, (uint16_t)((guid.data4 & 0xffff000000000000u) >> 48)
			, (uint16_t)((guid.data4 & 0x0000ffff00000000u) >> 32)
			, (uint32_t)((guid.data4 & 0x00000000ffffffffu) >>  0)
			);
		str.set(buf, sizeof(buf)-1);
	}

} // namespace GuidFn

} // namespace Rio
