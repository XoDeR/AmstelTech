#include "Core/CommandLine.h"

#include "Core/Strings/String.h"

namespace Rio
{

static bool getIsShortOption(const char* argumentsString, char shortOption)
{
	return shortOption != '\0'
		&& getStrLen32(argumentsString) > 1
		&& argumentsString[0] == '-'
		&& argumentsString[1] == shortOption
		;
}

static bool getIsLongOption(const char* argumentsString, const char* longOption)
{
	return longOption != nullptr
		&& getStrLen32(argumentsString) > 2
		&& argumentsString[0] == '-'
		&& argumentsString[1] == '-'
		&& strcmp(&argumentsString[2], longOption) == 0
		;
}

static int findOption(int argumentsCount, const char** argumentsVector, const char* longOption, char shortOption)
{
	for (int i = 0; i < argumentsCount; ++i)
	{
		if (getIsLongOption(argumentsVector[i], longOption) || getIsShortOption(argumentsVector[i], shortOption))
		{
			return i;
		}
	}

	return argumentsCount;
}

CommandLine::CommandLine(int argumentsCount, const char** argumentsVector)
	: argumentsCount(argumentsCount)
	, argumentsVector(argumentsVector)
{
}

const char* CommandLine::getParameter(int i, const char* longOption, char shortOption)
{
	int argumentsCount = findOption(this->argumentsCount, this->argumentsVector, longOption, shortOption);
	return argumentsCount + i < this->argumentsCount ? this->argumentsVector[argumentsCount + i + 1] : nullptr;
}

bool CommandLine::hasOption(const char* longOption, char shortOption)
{
	return findOption(this->argumentsCount, this->argumentsVector, longOption, shortOption) < this->argumentsCount;
}

} // namespace Rio
