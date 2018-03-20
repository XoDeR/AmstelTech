#pragma once

namespace Rio
{

struct CommandLine
{
	int argumentsCount = 0;
	const char** argumentsVector = nullptr;
	CommandLine(int argumentsCount, const char** argumentsVector);

	// Returns the i-th parameter of the option identified by <longOption> or <shortOption>
	// Returns nullptr if the parameter does not exist
	const char* getParameter(int i, const char* longOption, char shortOption = '\0');

	// Returns whether the command line has the option identified by <longOption> or <shortOption>
	bool hasOption(const char* longOption, char shortOption = '\0');
};

} // namespace Rio
