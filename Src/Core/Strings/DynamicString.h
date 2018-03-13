#pragma once

#include "Core/Containers/Array.h"
#include "Core/Error/Error.h"
#include "Core/Memory/Memory.h"
#include "Core/Strings/FixedString.h"
#include "Core/Strings/String.h"
#include "Core/Strings/StringId.h"

#include <cstring> // memmove

namespace Rio
{

// Dynamic array of characters
struct DynamicString
{
	ALLOCATOR_AWARE;

	Array<char> dataArray;

	DynamicString(Allocator& a);

	DynamicString& operator=(const DynamicString& ds);
	DynamicString& operator=(const char* str);
	DynamicString& operator=(const char c);
	DynamicString& operator=(const FixedString& fs);

	// Sets the string to <str>
	void set(const char* str, uint32_t len);

	// Reserves space for at least <n> characters
	void reserve(uint32_t n);

	// Returns the length of the string
	uint32_t getLength() const;

	// Returns whether the string is empty
	bool getIsEmpty() const;

	// Removes leading white-space characters from the string
	void trimLeft();

	// Removes trailing white-space characters from the string
	void trimRight();

	// Removes leading and trailing white-space characters from the string
	void trim();

	// Returns whether the string starts with <str>
	bool startsWith(const char* str) const;

	// Returns whether the string ends with <str>
	bool endsWith(const char* str) const;

	// Returns the StringId32 of the string
	StringId32 toStringId32() const;

	// Returns the string as a NULL-terminated C string
	const char* getCStr() const;
};

inline DynamicString::DynamicString(Allocator& a)
	: dataArray(a)
{
}

inline void DynamicString::set(const char* str, uint32_t len)
{
	ArrayFn::resize(dataArray, len);
	strncpy(ArrayFn::begin(dataArray), str, len);
}

inline DynamicString& operator+=(DynamicString& a, const DynamicString& b)
{
	ArrayFn::push(a.dataArray, ArrayFn::begin(b.dataArray), ArrayFn::getCount(b.dataArray));
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const char* str)
{
	RIO_ENSURE(nullptr != str);
	ArrayFn::push(a.dataArray, str, getStrLen32(str));
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const char c)
{
	ArrayFn::pushBack(a.dataArray, c);
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const FixedString& fs)
{
	ArrayFn::push(a.dataArray, fs.getCStr(), fs.getLength());
	return a;
}

inline DynamicString& DynamicString::operator=(const DynamicString& ds)
{
	dataArray = ds.dataArray;
	return *this;
}

inline DynamicString& DynamicString::operator=(const char* str)
{
	RIO_ENSURE(nullptr != str);
	ArrayFn::clear(dataArray);
	ArrayFn::push(dataArray, str, getStrLen32(str));
	return *this;
}

inline DynamicString& DynamicString::operator=(const char c)
{
	ArrayFn::clear(dataArray);
	ArrayFn::pushBack(dataArray, c);
	return *this;
}

inline DynamicString& DynamicString::operator=(const FixedString& fs)
{
	ArrayFn::clear(dataArray);
	ArrayFn::push(dataArray, fs.getCStr(), fs.getLength());
	return *this;
}

inline bool operator<(const DynamicString& a, const DynamicString& b)
{
	return strcmp(a.getCStr(), b.getCStr()) < 0;
}

inline bool operator==(const DynamicString& a, const DynamicString& b)
{
	return strcmp(a.getCStr(), b.getCStr()) == 0;
}

inline bool operator==(const DynamicString& a, const char* str)
{
	RIO_ENSURE(nullptr != str);
	return strcmp(a.getCStr(), str) == 0;
}

inline void DynamicString::reserve(uint32_t n)
{
	ArrayFn::reserve(dataArray, n);
}

inline uint32_t DynamicString::getLength() const
{
	return getStrLen32(getCStr());
}

inline bool DynamicString::getIsEmpty() const
{
	return getLength() == 0;
}

inline void DynamicString::trimLeft()
{
	const char* str = getCStr();
	const char* end = skipSpaces(str);

	const uint32_t len = getStrLen32(end);

	memmove(ArrayFn::begin(dataArray), end, len);
	ArrayFn::resize(dataArray, len);
}

inline void DynamicString::trimRight()
{
	char* str = (char*)getCStr();
	char* end = str + getStrLen32(str) - 1;

	while (end > str && isspace(*end))
	{
		--end;
	}

	*(end + 1) = '\0';
}

inline void DynamicString::trim()
{
	trimLeft();
	trimRight();
}

inline bool DynamicString::startsWith(const char* str) const
{
	RIO_ENSURE(nullptr != str);
	const uint32_t ml = getStrLen32(getCStr());
	const uint32_t sl = getStrLen32(str);
	return sl <= ml && strncmp(&dataArray[0], str, sl) == 0;
}

inline bool DynamicString::endsWith(const char* str) const
{
	RIO_ENSURE(nullptr != str);
	const uint32_t ml = getStrLen32(getCStr());
	const uint32_t sl = getStrLen32(str);
	return sl <= ml && strncmp(&dataArray[ml - sl], str, sl) == 0;
}

inline StringId32 DynamicString::toStringId32() const
{
	return StringId32(getCStr());
}

inline const char* DynamicString::getCStr() const
{
	Array<char>& data = const_cast<Array<char>&>(dataArray);
	ArrayFn::pushBack(data, '\0');
	ArrayFn::popBack(data);
	return ArrayFn::begin(dataArray);
}

} // namespace Rio
