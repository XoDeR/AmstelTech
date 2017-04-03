// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"
#include "Core/Strings/StringUtils.h"

namespace RioCore
{

struct CommandLine
{
    int argumentListCount;
    const char** argumentList;

	CommandLine(int argumentListCount, const char** argumentList)
		: argumentListCount(argumentListCount)
		, argumentList(argumentList)
	{
	}

	int findArgument(const char* longOpt, char shortOpt)
	{
		for (int i = 0; i < this->argumentListCount; ++i)
		{
			if (getIsLongOpt(this->argumentList[i], longOpt) || getIsShortOpt(this->argumentList[i], shortOpt))
			{
				return i;
			}
		}

		return this->argumentListCount;
	}

	bool getIsShortOpt(const char* arg, char shortOpt)
	{
		return shortOpt != '\0'
			&& getStringLength32(arg) > 1
			&& arg[0] == '-'
			&& arg[1] == shortOpt
			;
	}

	bool getIsLongOpt(const char* arg, const char* longOpt)
	{
		return longOpt != nullptr
			&& getStringLength32(arg) > 2
			&& arg[0] == '-'
			&& arg[1] == '-'
			&& strcmp(&arg[2], longOpt) == 0
			;
	}

	const char* getParameter(int i, const char* longOpt, char shortOpt = '\0')
	{
		int argumentListCount = findArgument(longOpt, shortOpt);
		return argumentListCount + i < this->argumentListCount ? this->argumentList[argumentListCount + i + 1] : nullptr;
	}

	bool hasArgument(const char* longOpt, char shortOpt = '\0')
	{
		return findArgument(longOpt, shortOpt) < this->argumentListCount;
	}

    const char* findOption(const char* longOpt, const char* _default) const
    {
        const char* result = find('\0', longOpt, 1);
        return result == nullptr ? _default : result;
    }

    const char* findOption(const char shortOpt, const char* longOpt, const char* _default) const
    {
        const char* result = find(shortOpt, longOpt, 1);
        return result == nullptr ? _default : result;
    }

    const char* findOption(const char* longOpt, int _numParams = 1) const
    {
        const char* result = find('\0', longOpt, _numParams);
        return result;
    }

    const char* findOption(const char shortOpt, const char* longOpt = nullptr, int _numParams = 1) const
    {
        const char* result = find(shortOpt, longOpt, _numParams);
        return result;
    }

    bool hasArg(const char _short, const char* longOpt = nullptr) const
    {
        const char* arg = findOption(_short, longOpt, 0);
        return nullptr != arg;
    }

    bool hasArg(const char* longOpt) const
    {
        const char* arg = findOption('\0', longOpt, 0);
        return nullptr != arg;
    }

    bool hasArg(const char*& value, const char shortOpt, const char* longOpt = nullptr) const
    {
        const char* arg = findOption(shortOpt, longOpt, 1);
        value = arg;
        return nullptr != arg;
    }

    bool hasArg(int& _value, const char shortOpt, const char* longOpt = nullptr) const
    {
        const char* arg = findOption(shortOpt, longOpt, 1);
        if (nullptr != arg)
        {
            _value = atoi(arg);
            return true;
        }

        return false;
    }

    bool hasArg(unsigned int& _value, const char shortOpt, const char* longOpt = nullptr) const
    {
        const char* arg = findOption(shortOpt, longOpt, 1);
        if (nullptr != arg)
        {
            _value = atoi(arg);
            return true;
        }

        return false;
    }

    bool hasArg(float& _value, const char shortOpt, const char* longOpt = nullptr) const
    {
        const char* arg = findOption(shortOpt, longOpt, 1);
        if (nullptr != arg)
        {
            _value = float(atof(arg));
            return true;
        }

        return false;
    }

    bool hasArg(double& _value, const char shortOpt, const char* longOpt = nullptr) const
    {
        const char* arg = findOption(shortOpt, longOpt, 1);
        if (nullptr != arg)
        {
            _value = atof(arg);
            return true;
        }

        return false;
    }

    bool hasArg(bool& boolValue, const char shortParam, const char* longParam = nullptr) const
    {
        const char* arg = findOption(shortParam, longParam, 1);
        if (nullptr != arg)
        {
            if ('0' == *arg || (0 == stricmp(arg, "false")))
            {
                boolValue = false;
            }
            else if ('0' != *arg || (0 == stricmp(arg, "true")))
            {
                boolValue = true;
            }

            return true;
        }

        return false;
    }

private:
    const char* find(const char shortParam, const char* longParam, int paramsCount) const
    {
        for (int ii = 0; ii < argumentListCount; ++ii)
        {
            const char* arg = argumentList[ii];
            if ('-' == *arg)
            {
                ++arg;
                if (shortParam == *arg)
                {
                    if (1 == strlen(arg))
                    {
                        if (0 == paramsCount)
                        {
                            return "";
                        }
                        else if (ii + paramsCount < argumentListCount
                            && '-' != *argumentList[ii + 1])
                        {
                            return argumentList[ii + 1];
                        }

                        return NULL;
                    }
                }
                else if (nullptr != longParam
                    &&  '-' == *arg
                    && 0 == stricmp(arg + 1, longParam))
                {
                    if (0 == paramsCount)
                    {
                        return "";
                    }
                    else if (ii + paramsCount < argumentListCount
                        &&  '-' != *argumentList[ii + 1])
                    {
                        return argumentList[ii + 1];
                    }

                    return nullptr;
                }
            }
        }

        return nullptr;
    }
};

} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka