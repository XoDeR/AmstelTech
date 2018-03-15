#include "Core/Json/RJson.h"

#include "Core/Containers/Map.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Strings/String.h"

namespace Rio
{

namespace RJsonFn
{
	static const char* skipExpectedToNext(const char* json, const char c = 0)
	{
		RIO_ENSURE(nullptr != json);

		if (c && c != *json)
		{
			RIO_ASSERT(false, "Expected '%c' got '%c'", c, *json);
		}

		return ++json;
	}

	static const char* skipString(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		while (*++json)
		{
			if (*json == '"')
			{
				++json;
				return json;
			}
			else if (*json == '\\')
			{
				++json;
			}
		}

		return json;
	}

	static const char* skipValue(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		switch (*json)
		{
		case '"':
			json = skipString(json);
			if (*json == '"')
			{
				++json;
				json = strstr(json, "\"\"\"");
				RIO_ENSURE(json);
				++json;
				++json;
				++json;
			}
			break;

		case '[': json = skipBlock(json, '[', ']'); break;
		case '{': json = skipBlock(json, '{', '}'); break;
		default: for (; *json != '\0' && *json != ',' && *json != '\n' && *json != ' ' && *json != '}' && *json != ']'; ++json); break;
		}

		return json;
	}

	static const char* skipComments(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		if (*json == '/')
		{
			++json;
			if (*json == '/')
			{
				json = skipExpectedToNext(json, '/');
				while (*json && *json != '\n')
				{
					++json;
				}
			}
			else if (*json == '*')
			{
				++json;
				while (*json && *json != '*')
				{
					++json;
				}
				json = skipExpectedToNext(json, '*');
				json = skipExpectedToNext(json, '/');
			}
			else
			{
				RIO_FATAL("Bad comment");
			}
		}

		return json;
	}

	static const char* skipSpaces(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		while (*json)
		{
			if (*json == '/')
			{
				json = skipComments(json);
			}
			else if (isspace(*json) || *json == ',')
			{
				++json;
			}
			else
			{
				break;
			}
		}

		return json;
	}

	JsonValueType::Enum getJsonType(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		switch (*json)
		{
		case '"': return JsonValueType::STRING;
		case '{': return JsonValueType::OBJECT;
		case '[': return JsonValueType::ARRAY;
		case '-': return JsonValueType::NUMBER;
		default: return (isdigit(*json)) ? JsonValueType::NUMBER : (*json == 'n' ? JsonValueType::NIL : JsonValueType::BOOL);
		}
	}

	static const char* parseKey(const char* json, DynamicString& key)
	{
		RIO_ENSURE(nullptr != json);
		if (*json == '"')
		{
			parseString(json, key);
			return skipString(json);
		}

		while (true)
		{
			if (isspace(*json) || *json == '=' || *json == ':')
			{
				return json;
			}

			key += *json++;
		}

		RIO_FATAL("Bad key");
		return nullptr;
	}

	static double parseNumber(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		TempAllocator512 tempAllocator512;
	 	Array<char> number(tempAllocator512);

		if (*json == '-')
		{
			ArrayFn::pushBack(number, '-');
			++json;
		}
		while (isdigit(*json))
		{
			ArrayFn::pushBack(number, *json);
			++json;
		}

		if (*json == '.')
		{
			ArrayFn::pushBack(number, '.');
			while (*++json && isdigit(*json))
			{
				ArrayFn::pushBack(number, *json);
			}
		}

		if (*json == 'e' || *json == 'E')
		{
			ArrayFn::pushBack(number, *json);
			++json;

			if (*json == '-' || *json == '+')
			{
				ArrayFn::pushBack(number, *json);
				++json;
			}
			while (isdigit(*json))
			{
				ArrayFn::pushBack(number, *json);
				++json;
			}
		}

		ArrayFn::pushBack(number, '\0');

		double result = 0.0;
		int ok = sscanf(ArrayFn::begin(number), "%lf", &result);
		RIO_ASSERT(ok == 1, "Failed to parse double: %s", ArrayFn::begin(number));
		RIO_UNUSED(ok);
		return result;
	}

	int32_t parseInt32(const char* json)
	{
		return (int32_t)parseNumber(json);
	}

	float parseFloat(const char* json)
	{
		return (float)parseNumber(json);
	}

	bool parseBool(const char* json)
	{
		RIO_ENSURE(nullptr != json);

		switch (*json)
		{
		case 't':
			json = skipExpectedToNext(json, 't');
			json = skipExpectedToNext(json, 'r');
			json = skipExpectedToNext(json, 'u');
			skipExpectedToNext(json, 'e');
			return true;

		case 'f':
			json = skipExpectedToNext(json, 'f');
			json = skipExpectedToNext(json, 'a');
			json = skipExpectedToNext(json, 'l');
			json = skipExpectedToNext(json, 's');
			skipExpectedToNext(json, 'e');
			return false;

		default:
			RIO_FATAL("Bad boolean");
			return false;
		}
	}

	void parseString(const char* json, DynamicString& string)
	{
		RIO_ENSURE(nullptr != json);

		if (*json == '"')
		{
			while (*++json)
			{
				// Empty string
				if (*json == '"')
				{
					++json;
					return;
				}
				else if (*json == '\\')
				{
					++json;

					switch (*json)
					{
					case '"': string += '"'; break;
					case '\\': string += '\\'; break;
					case '/': string += '/'; break;
					case 'b': string += '\b'; break;
					case 'f': string += '\f'; break;
					case 'n': string += '\n'; break;
					case 'r': string += '\r'; break;
					case 't': string += '\t'; break;
					default: RIO_FATAL("Bad escape character"); break;
					}
				}
				else
				{
					string += *json;
				}
			}
		}

		RIO_FATAL("Bad string");
	}

	void parseArray(const char* json, JsonArray& array)
	{
		RIO_ENSURE(nullptr != json);

		if (*json == '[')
		{
			json = skipSpaces(++json);

			if (*json == ']')
			{
				return;
			}

			while (*json)
			{
				ArrayFn::pushBack(array, json);

				json = skipValue(json);
				json = skipSpaces(json);

				if (*json == ']')
				{
					return;
				}

				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad array");
	}

	static void parseRootObject(const char* json, JsonObject& object)
	{
		RIO_ENSURE(nullptr != json);

		while (*json)
		{
			const char* keyBegin = *json == '"' ? (json + 1) : json;

			TempAllocator256 ta;
			DynamicString key(ta);
			json = parseKey(json, key);

			FixedString keyStr(keyBegin, key.getLength());

			json = skipSpaces(json);
			json = skipExpectedToNext(json, (*json == '=') ? '=' : ':');
			json = skipSpaces(json);

			MapFn::set(object.jsonMap, keyStr, json);

			json = skipValue(json);
			json = skipSpaces(json);
		}
	}

	void parseObject(const char* json, JsonObject& object)
	{
		RIO_ENSURE(nullptr != json);

		if (*json == '{')
		{
			json = skipSpaces(++json);

			if (*json == '}')
			{
				return;
			}

			while (*json)
			{
				const char* keyBegin = *json == '"' ? (json + 1) : json;

				TempAllocator256 ta;
				DynamicString key(ta);
				json = parseKey(json, key);

				FixedString keyStr(keyBegin, key.getLength());

				json = skipSpaces(json);
				json = skipExpectedToNext(json, (*json == '=') ? '=' : ':');
				json = skipSpaces(json);

				MapFn::set(object.jsonMap, keyStr, json);

				json = skipValue(json);
				json = skipSpaces(json);

				if (*json == '}')
				{
					return;
				}

				json = skipSpaces(json);
			}
		}

		RIO_FATAL("Bad object");
	}

	void parse(const char* json, JsonObject& object)
	{
		RIO_ENSURE(nullptr != json);

		json = skipSpaces(json);

		if (*json == '{')
		{
			parseObject(json, object);
		}
		else
		{
			parseRootObject(json, object);
		}
	}

	void parse(Buffer& json, JsonObject& object)
	{
		ArrayFn::pushBack(json, '\0');
		ArrayFn::popBack(json);
		parse(ArrayFn::begin(json), object);
	}

} // namespace RJsonFn

namespace RJsonFn
{
	Vector2 parseVector2(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		RJsonFn::parseArray(json, array);

		Vector2 v;
		v.x = RJsonFn::parseFloat(array[0]);
		v.y = RJsonFn::parseFloat(array[1]);
		return v;
	}

	Vector3 parseVector3(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		RJsonFn::parseArray(json, array);

		Vector3 v;
		v.x = RJsonFn::parseFloat(array[0]);
		v.y = RJsonFn::parseFloat(array[1]);
		v.z = RJsonFn::parseFloat(array[2]);
		return v;
	}

	Vector4 parseVector4(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		RJsonFn::parseArray(json, array);

		Vector4 v;
		v.x = RJsonFn::parseFloat(array[0]);
		v.y = RJsonFn::parseFloat(array[1]);
		v.z = RJsonFn::parseFloat(array[2]);
		v.w = RJsonFn::parseFloat(array[3]);
		return v;
	}

	Quaternion parseQuaternion(const char* json)
	{
		TempAllocator64 ta;
		JsonArray array(ta);
		RJsonFn::parseArray(json, array);

		Quaternion q;
		q.x = RJsonFn::parseFloat(array[0]);
		q.y = RJsonFn::parseFloat(array[1]);
		q.z = RJsonFn::parseFloat(array[2]);
		q.w = RJsonFn::parseFloat(array[3]);
		return q;
	}

	Matrix4x4 parseMatrix4x4(const char* json)
	{
		TempAllocator256 ta;
		JsonArray array(ta);
		RJsonFn::parseArray(json, array);

		Matrix4x4 m;
		m.x.x = RJsonFn::parseFloat(array[0]);
		m.x.y = RJsonFn::parseFloat(array[1]);
		m.x.z = RJsonFn::parseFloat(array[2]);
		m.x.w = RJsonFn::parseFloat(array[3]);

		m.y.x = RJsonFn::parseFloat(array[4]);
		m.y.y = RJsonFn::parseFloat(array[5]);
		m.y.z = RJsonFn::parseFloat(array[6]);
		m.y.w = RJsonFn::parseFloat(array[7]);

		m.z.x = RJsonFn::parseFloat(array[8]);
		m.z.y = RJsonFn::parseFloat(array[9]);
		m.z.z = RJsonFn::parseFloat(array[10]);
		m.z.w = RJsonFn::parseFloat(array[11]);

		m.t.x = RJsonFn::parseFloat(array[12]);
		m.t.y = RJsonFn::parseFloat(array[13]);
		m.t.z = RJsonFn::parseFloat(array[14]);
		m.t.w = RJsonFn::parseFloat(array[15]);
		return m;
	}

	StringId32 parseStringId(const char* json)
	{
		TempAllocator256 ta;
		DynamicString str(ta);
		RJsonFn::parseString(json, str);
		return str.toStringId32();
	}

	ResourceId parseResourceId(const char* json)
	{
		TempAllocator256 ta;
		DynamicString str(ta);
		RJsonFn::parseString(json, str);
		return ResourceId(str.getCStr());
	}

	Guid parseGuid(const char* json)
	{
		TempAllocator64 ta;
		DynamicString str(ta);
		RJsonFn::parseString(json, str);
		return GuidFn::parseFromString(str.getCStr());
	}

	void parseVerbatim(const char* json, DynamicString& string)
	{
		RIO_ENSURE(nullptr != json);

		json = skipExpectedToNext(json, '"');
		json = skipExpectedToNext(json, '"');
		json = skipExpectedToNext(json, '"');

		const char* end = strstr(json, "\"\"\"");
		RIO_ASSERT(end, "Bad verbatim string");

		string.set(json, uint32_t(end - json));
	}

} // namespace JsonFn

} // namespace Rio
