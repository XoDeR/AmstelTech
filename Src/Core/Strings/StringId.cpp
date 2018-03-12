#include "Core/Strings/StringId.h"

#include "Core/Error/Error.h"
#include "Core/Murmur.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/String.h"

#include <inttypes.h> // PRIx64

namespace Rio
{

StringId32::StringId32(const char* str)
{
	hashString(str, getStrLen32(str));
}

StringId32::StringId32(const char* str, uint32_t len)
{
	hashString(str, len);
}

void StringId32::hashString(const char* str, uint32_t len)
{
	RIO_ENSURE(nullptr != str);
	id = murmur32(str, len, 0);
}

void StringId32::toString(DynamicString& s)
{
	char buf[8+1];
	Rio::snPrintF(buf, sizeof(buf), "%.8x", id);
	s.set(buf, sizeof(buf)-1);
}

StringId64::StringId64(const char* str)
{
	hashString(str, getStrLen32(str));
}

StringId64::StringId64(const char* str, uint32_t len)
{
	hashString(str, len);
}

void StringId64::hashString(const char* str, uint32_t len)
{
	RIO_ENSURE(nullptr != str);
	id = murmur64(str, len, 0);
}

void StringId64::toString(DynamicString& s)
{
	char buf[16+1];
	Rio::snPrintF(buf, sizeof(buf), "%.16" PRIx64, id);
	s.set(buf, sizeof(buf)-1);
}

} // namespace Rio
