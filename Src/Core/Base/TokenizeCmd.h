#pragma once

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

namespace RioCore
{
	static inline const char* tokenizeCommandLine(const char* commandLine, char* _buffer, uint32_t& _bufferSize, int& _argc, char* _argv[], int _maxArgvs, char _term = '\0')
	{
		int argc = 0;
		const char* curr = commandLine;
		char* currOut = _buffer;
		char term = ' ';
		bool sub = false;
		
		enum ParserState
		{
			SkipWhitespace,
			SetTerm,
			Copy,
			Escape,
			End,
		};

		ParserState state = SkipWhitespace;
		
		while ('\0' != *curr && _term != *curr && argc < _maxArgvs)
		{
			switch (state)
			{
				case SkipWhitespace:
					for (; isspace(*curr); ++curr) {}; // skip whitespace
					state = SetTerm;
					break;
					
				case SetTerm:
					if ('"' == *curr)
					{
						term = '"';
						++curr; // skip begining quote
					}
					else
					{
						term = ' ';
					}
					
					_argv[argc] = currOut;
					++argc;
					
					state = Copy;
					break;
					
				case Copy:
					if ('\\' == *curr)
					{
						state = Escape;
					}
					else if ('"' == *curr
						&&  '"' != term)
					{
						sub = !sub;
					}
					else if (isspace(*curr) && !sub)
					{
						state = End;
					}
					else if (term != *curr || sub)
					{
						*currOut = *curr;
						++currOut;
					}
					else
					{
						state = End;
					}
					++curr;
					break;
					
				case Escape:
					{
						const char* start = --curr;
						for (; '\\' == *curr; ++curr) {};

						if ('"' != *curr)
						{
							int count = (int)(curr-start);

							curr = start;
							for (int ii = 0; ii < count; ++ii)
							{
								*currOut = *curr;
								++currOut;
								++curr;
							}
						}
						else
						{
							curr = start+1;
							*currOut = *curr;
							++currOut;
							++curr;
						}
					}
					state = Copy;
					break;
					
				case End:
					*currOut = '\0';
					++currOut;
					state = SkipWhitespace;
					break;
			}
		}
		
		*currOut = '\0';
		if (0 < argc
		&&  '\0' == _argv[argc-1][0])
		{
			--argc;
		}

		_bufferSize = (uint32_t)(currOut - _buffer);
		_argc = argc;

		if ('\0' != *curr)
		{
			++curr;
		}

		return curr;
	}

} // namespace RioCore
