#pragma once

#include "Core/Containers/Array.h"
#include "Core/Strings/String.h"
#include "Core/Strings/Types.h"

namespace Rio
{

namespace StringStreamFn
{
	// Returns the stream as a NULL terminated C string
	const char* getCStr(StringStream& s);

	template <typename T> StringStream& streamPrintF(StringStream& s, const char* format, T& val);

} // namespace StringStreamFn

inline StringStream& operator<<(StringStream& s, char val)
{
	ArrayFn::pushBack(s, val);
	return s;
}

inline StringStream& operator<<(StringStream& s, int16_t val)
{
	return StringStreamFn::streamPrintF(s, "%hd", val);
}

inline StringStream& operator<<(StringStream& s, uint16_t val)
{
	return StringStreamFn::streamPrintF(s, "%hu", val);
}

inline StringStream& operator<<(StringStream& s, int32_t val)
{
	return StringStreamFn::streamPrintF(s, "%d", val);
}

inline StringStream& operator<<(StringStream& s, uint32_t val)
{
	return StringStreamFn::streamPrintF(s, "%u", val);
}

inline StringStream& operator<<(StringStream& s, int64_t val)
{
	return StringStreamFn::streamPrintF(s, "%lld", val);
}

inline StringStream& operator<<(StringStream& s, uint64_t val)
{
	return StringStreamFn::streamPrintF(s, "%llu", val);
}

inline StringStream& operator<<(StringStream& s, float val)
{
	return StringStreamFn::streamPrintF(s, "%g", val);
}

inline StringStream& operator<<(StringStream& s, double val)
{
	return StringStreamFn::streamPrintF(s, "%g", val);
}

inline StringStream& operator<<(StringStream& s, const char* str)
{
	ArrayFn::push(s, str, getStrLen32(str));
	return s;
}

namespace StringStreamFn
{
	inline const char* getCStr(StringStream& s)
	{
		ArrayFn::pushBack(s, '\0');
		ArrayFn::popBack(s);
		return ArrayFn::begin(s);
	}

	template <typename T>
	inline StringStream& streamPrintF(StringStream& s, const char* format, T& val)
	{
		char buf[64];
		Rio::snPrintF(buf, sizeof(buf), format, val);
		return s << buf;
	}

} // namespace StringStreamFn

} // namespace Rio
