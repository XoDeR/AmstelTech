#include "Resource/Sprite/ExpressionLanguage.h"

#include "Core/Error/Error.h"
#include "Core/Math/Math.h"

#include <alloca.h> // TODO memory allocation

#include <cstring>
#include <limits.h>
#include <stdlib.h>

namespace Rio
{

namespace ExpressionLanguageFn 
{

	// Byte code constants
	// If the upper 12 bits of the byte code do not match one of these values, the operation is BYTE_CODE_PUSH_FLOAT and the byte code specify the 32 bit float to push
	// If the upper 12 bits match one of these values (which are all NaNs, so they should never appear as regular floats), the operation will instead be the one matching
	// The remaining 20 bits of the byte code are used for the id of functions and variables
	enum ByteCode 
	{
		BYTE_CODE_FUNCTION = 0x7f800000, 
		BYTE_CODE_PUSH_VARIABLE = 0x7f900000, 
		BYTE_CODE_END = 0x7fa00000
	};

	// Returns the byte code operation part of the byte code word
	static inline uint32_t getByteCodeOperation(uint32_t byteCodeValueCombined)
	{
		return byteCodeValueCombined & 0xfff00000; // byte code operation mask 0xfff00000
	}

	// Returns the id part of the byte code word
	static inline uint32_t getByteCodeIdPart(uint32_t byteCodeValueCombined)
	{
		return byteCodeValueCombined & 0x000fffff; // byte code id mask 0x000fffff
	}

	// Returns true if the byte code word is a BYTE_CODE_PUSH_FLOAT operation.
	static inline bool getIsByteCodePushFloat(uint32_t byteCodeValueCombined)
	{
		return (byteCodeValueCombined & 0x7f80000) != 0x7f8;
	}

	// Opcodes for functions
	enum OpCode 
	{
		OP_ADD, 
		OP_SUB, 
		OP_MUL, 
		OP_DIV, 
		OP_UNARY_MINUS, 
		OP_NOP, 
		OP_SIN, 
		OP_COS, 
		OP_ABS, 
		OP_MATCH, 
		OP_MATCH2D
	};

	static inline float pop(Stack& stack) 			
	{
		RIO_ASSERT(stack.size > 0, "Stack underflow"); 
		return stack.data[--stack.size];
	}

	static inline void push(Stack& stack, float floatValue)	
	{
		RIO_ASSERT(stack.size < stack.capacity, "Stack overflow"); 
		stack.data[stack.size++] = floatValue;
	}

	inline float getMaxFloat(float a, float b)
	{
		return a > b ? a : b;
	}

	inline float getLength(float a, float b)
	{
		return getSqrtFloat((b - a) * (b - a));
	}

	inline float match(float a, float b)
	{
		return getMaxFloat(1.0f - getLength(a, b), 0.0f);
	}

	inline float match2d(float a, float b, float c, float d)
	{
		return getMaxFloat(1.0f - getLength(a, b), 0.0f);
	}

	// Computes the function specified by <operationCode> on the <stack>
	static inline void computeFunction(OpCode operationCode, Stack& stack)
	{
		#define POP() pop(stack)
		#define PUSH(floatValue) push(stack, floatValue)

		float a = 0.0f;
		float b = 0.0f;
		float c = 0.0f;
		float d = 0.0f;

		switch (operationCode)
		{
			case OP_ADD: 
				b = POP(); 
				a = POP(); 
				PUSH(a + b); 
				break;
			case OP_SUB: 
				b = POP(); 
				a = POP(); 
				PUSH(a - b); 
				break;
			case OP_MUL: 
				b = POP(); 
				a = POP(); 
				PUSH(a * b); 
				break;
			case OP_DIV: 
				b = POP(); 
				a = POP(); 
				PUSH(a / b); 
				break;
			case OP_UNARY_MINUS: 
				PUSH(-POP()); 
				break;
			case OP_SIN: 
				PUSH(getSinFloat(POP())); 
				break;
			case OP_COS: 
				PUSH(getCosFloat(POP())); 
				break;
			case OP_ABS: 
				a = POP(); 
				PUSH(getAbsFloat(a)); 
				break;
			case OP_MATCH: 
				b = POP(); 
				a = POP(); 
				PUSH(match(a, b));
				break;
			case OP_MATCH2D: 
				d = POP(); 
				c = POP(); 
				b = POP(); 
				a = POP(); 
				PUSH(match2d(a, b, c, d)); 
				break;
			case OP_NOP: 
				break;
			default:
				RIO_FATAL("Unknown opcode");
		}

		#undef POP
		#undef PUSH
	}

	// Union to cast through to convert between float and unsigned
	union FloatAndUnsigned
	{
		float floatValue;
		uint32_t uint32Value;
	};

	static inline float getFloatFromUint32(uint32_t uint32Value)
	{
		FloatAndUnsigned floatAndUnsigned;
		floatAndUnsigned.uint32Value = uint32Value;
		return floatAndUnsigned.floatValue;
	}

	#ifdef CAN_COMPILE

	static inline uint32_t getUint32FromFloat(float floatValue)
	{
		FloatAndUnsigned floatAndUnsigned;
		floatAndUnsigned.floatValue = floatValue;
		return floatAndUnsigned.uint32Value;
	}

	#ifdef WIN32
		float strtof(const char* stringPtr, char** endPtr) 
		{
			return (float)strtod(stringPtr, endPtr);
		}
	#endif // WIN32

	// Represents a token in the expression language
	// The tokens are used both during the tokenization phase and as a representation of the program
	// (A list of tokens in reverse polish notation form)
	struct Token
	{
		enum TokenType 
		{
			EMPTY, 
			NUMBER, 
			FUNCTION, 
			VARIABLE, 
			LEFT_PARENTHESIS, 
			RIGHT_PARENTHESIS
		};

		Token() 
		{
		}

		Token(TokenType type) 
			: type(type) 
		{
		}

		Token(TokenType type, uint32_t id)
			: type(type)
			, id(id) 
		{
		}

		Token(TokenType type, float value) 
			: type(type)
			, value(value) 
		{
		}

		TokenType type;	// Identifies the type of the token

		union 
		{
			uint32_t id; // Id for FUNCTION and VARIABLE tokens
			float value; // Numeric value for NUMBER tokens
		};
	};

	// Describes a function
	struct Function
	{
		OpCode operationCode = OpCode::OP_NOP;
		uint32_t precedence = 0; // The precedence of the function operator
		uint32_t arity = 0; // The number of arguments that the function takes

		Function() 
		{
		}

		Function(OpCode operationCode, uint32_t precedence, uint32_t arity)
			: operationCode(operationCode)
			, precedence(precedence)
			, arity(arity) 
		{
		}
	};

	// Represents the environment in which we are compiling -- the available variables, constants and functions
	struct CompileEnvironment
	{
		uint32_t variableListCount = 0;
		const char** variableNameList = nullptr;

		uint32_t constantListCount = 0;
		const char** constantNameList = nullptr;
		const float* constantValueList = nullptr;

		uint32_t functionListCount = 0;
		const char** functionNameList = nullptr;
		const Function* functionValueList = nullptr;

		// Finds a string in <strings> matching <stringToMatch> of length <length> and returns its index
		// Returns UINT32_MAX if no such string is found
		static uint32_t findIndexOfString(const char* stringToMatch, uint32_t length, uint32_t stringListCount, const char** stringList)
		{
			for (uint32_t i = 0; i < stringListCount; ++i)
			{
				if (strncmp(stringToMatch, stringList[i], length) == 0 && strlen(stringList[i]) == length)
				{
					return i;
				}
			}
			return UINT32_MAX;
		}

		// Finds a token representing the identifier in the environment
		Token getTokenByIdentifier(const char* identifier, uint32_t length) const
		{
			uint32_t i = UINT32_MAX;
			if ((i = findIndexOfString(identifier, length, variableListCount, variableNameList)) != UINT_MAX)
			{
				return Token(Token::VARIABLE, i);
			}
			else if ((i = findIndexOfString(identifier, length, constantListCount, constantNameList)) != UINT_MAX)
			{
				return Token(Token::NUMBER, constantValueList[i]);
			}
			else if ((i = findIndexOfString(identifier, length, functionListCount, functionNameList)) != UINT_MAX)
			{
				return Token(Token::FUNCTION, i);
			}
			else 
			{
				RIO_FATAL("Unknown identifier: %s", identifier);
				return Token();
			}
		}

		// Finds a token representing the identifier in the environment
		Token getTokenByIdentifier(const char* identifier) const
		{
			return getTokenByIdentifier(identifier, (uint32_t)(strlen(identifier)));
		}

		// True if there is a function matching the specified identifier
		bool hasFunctionWithIdentifier(char* identifier) const
		{
			return findIndexOfString(identifier, (uint32_t)(strlen(identifier)), functionListCount, functionNameList) != UINT_MAX;
		}
	};

	// Tokenizes the source code <p> into a sequence of tokens
	// The environment <compileEnvironment> is used for looking up source code identifiers
	// Returns the total number of tokens
	// If the returned number is greater than the <capacity>, only the first <capacity> items will be converted
	static uint32_t tokenize(const char* sourceCodePointer, const CompileEnvironment& compileEnvironment, Token* tokenList, uint32_t capacity)
	{
		// Determines if the next + or - is a binary or unary operator
		bool binary = false;

		uint32_t tokenListCount = 0;
		uint32_t tokenListOverflow = 0;

		while (*sourceCodePointer != 0)
		{
			Token token(Token::EMPTY);

			// Numbers
			if (*sourceCodePointer >= '0' && *sourceCodePointer <= '9')
			{
				char* out = nullptr;
				token = Token(Token::NUMBER, strtof(sourceCodePointer, &out));
				sourceCodePointer = out;
				binary = true;
			// Identifiers
			} 
			else if ((*sourceCodePointer >= 'a' && *sourceCodePointer <= 'z') || (*sourceCodePointer >= 'A' && *sourceCodePointer <= 'Z') || (*sourceCodePointer == '_'))
			{
				const char* identifier = sourceCodePointer;
				while ((*sourceCodePointer >= 'a' && *sourceCodePointer <= 'z') 
					|| (*sourceCodePointer >= 'A' && *sourceCodePointer <= 'Z') 
					|| (*sourceCodePointer == '_') 
					|| (*sourceCodePointer >= '0' && *sourceCodePointer <= '9'))
				{
					sourceCodePointer++;
				}
				token = compileEnvironment.getTokenByIdentifier(identifier, (uint32_t)(sourceCodePointer - identifier));
				binary = true;
			// Operators
			}
			else 
			{
				switch (*sourceCodePointer)
				{
					case '(': 
						token = Token(Token::LEFT_PARENTHESIS); 
						binary = false; 
						break;
					case ')': 
						token = Token(Token::RIGHT_PARENTHESIS); 
						binary = true; 
						break;
					case ' ': 
					case '\t': 
					case '\n': 
					case '\r': 
						break;
					case '-': 
						token = compileEnvironment.getTokenByIdentifier(binary ? "-" : "u-"); 
						binary = false; 
						break;
					case '+': 
						token = compileEnvironment.getTokenByIdentifier(binary ? "+" : "u+"); 
						binary = false; 
						break;

					default: 
					{
						char firstString[2] = { *sourceCodePointer, 0 };
						char secondString[3] = { *sourceCodePointer, *(sourceCodePointer + 1), 0 };

						if (secondString[1] && compileEnvironment.hasFunctionWithIdentifier(secondString))
						{
							token = compileEnvironment.getTokenByIdentifier(secondString);
							++sourceCodePointer;
						} 
						else
						{
							token = compileEnvironment.getTokenByIdentifier(firstString);
						}

						binary = false;
						break;
					}
				}
				++sourceCodePointer;
			}

			if (token.type != Token::EMPTY) 
			{
				if (tokenListCount == capacity)
				{
					++tokenListOverflow;
				}
				else
				{
					tokenList[tokenListCount++] = token;
				}
			}
		}

		return tokenListCount + tokenListOverflow;
	}

	// Performs constant folding on the program represented by <reversePolishTokenList> which is a sequence of tokens in reverse polish notation
	// Any function found in the token stream which only takes constant arguments is replaced by the result of evaluating the function over the constant arguments
	static void foldConstants(Token* reversePolishTokenList, uint32_t& tokenListCount, CompileEnvironment& compileEnvironment)
	{
		static const int MAX_ARITY = 4;
		float stackData[MAX_ARITY];

		for (uint32_t i = 0; i < tokenListCount; ++i)
		{
			if (reversePolishTokenList[i].type != Token::FUNCTION)
			{
				continue;
			}

			Stack stack(stackData, MAX_ARITY);
			bool hasConstantArgumentList = true;
			Function function = compileEnvironment.functionValueList[reversePolishTokenList[i].id];
			uint32_t arity = function.arity;
			RIO_ASSERT(arity <= MAX_ARITY, "MAX_ARITY too small");
			RIO_ASSERT(i >= arity, "Too few arguments to function");
			uint32_t argumentListStart = i - arity;
			for (uint32_t j = 0; j < arity && hasConstantArgumentList; ++j)
			{
				hasConstantArgumentList = hasConstantArgumentList && reversePolishTokenList[i - j - 1].type == Token::NUMBER;
				stack.data[j] = reversePolishTokenList[argumentListStart + j].value;
			}
			if (hasConstantArgumentList == false)
			{
				continue;
			}

			stack.size = arity;
			computeFunction(function.operationCode, stack);
			uint32_t resultListIndex = stack.size;

			int32_t toRemove = int32_t(arity + 1) - int32_t(resultListIndex);
			if (toRemove > 0) 
			{
				memmove(&reversePolishTokenList[argumentListStart], &reversePolishTokenList[argumentListStart + toRemove], sizeof(Token)*(tokenListCount - argumentListStart - toRemove));
				tokenListCount -= toRemove;
			}
			for (uint32_t constantResult = 0; constantResult < stack.size; ++constantResult)
			{
				reversePolishTokenList[argumentListStart + constantResult] = Token(Token::NUMBER, stack.data[constantResult]);
			}
			i = argumentListStart - 1;
		}
	}

	// Generates bytecode from a program in reverse Polish notation token stream form
	// Returns the number of byte code tokens generated. If the returned number is > capacity, only the first capacity items are generated
	static uint32_t generateByteCode(Token* reversePolishTokenList, uint32_t tokenListCount, const CompileEnvironment& compileEnvironment, uint32_t* byteCode, uint32_t capacity)
	{
		uint32_t size = 0;
		uint32_t overflow = 0;

		for (uint32_t i = 0; i < tokenListCount; ++i)
		{
			Function function;
			Token token = reversePolishTokenList[i];
			uint32_t currentByteCode = 0;
			switch (token.type)
			{
				case Token::NUMBER:
					currentByteCode = getUint32FromFloat(token.value);
					break;
				case Token::VARIABLE:
					currentByteCode = BYTE_CODE_PUSH_VARIABLE + token.id;
					break;
				case Token::FUNCTION:
					function = compileEnvironment.functionValueList[token.id];
					currentByteCode = BYTE_CODE_FUNCTION + function.operationCode;
					break;
				default:
					RIO_FATAL("Unknown token");
					break;
			}
			if (size < capacity)
			{
				byteCode[size++] = currentByteCode;
			}
			else
			{
				++overflow;
			}
		}

		uint32_t byteCodeEndMarker = BYTE_CODE_END;
		if (size < capacity)
		{
			byteCode[size++] = byteCodeEndMarker;
		}
		else
		{
			++overflow;
		}

		return size + overflow;
	}

	// Represents an item on the function call stack
	// This object is comparable so that functions that have higher precedence are executed before others
	struct FunctionStackItem
	{
		Token token;
		int32_t precedence = 0;
		int32_t parenthesisLevel = 0;

		FunctionStackItem() 
		{
		}

		FunctionStackItem(Token token, int32_t precedence, int32_t parenthesisLevel)
			: token(token)
			, precedence(precedence)
			, parenthesisLevel(parenthesisLevel)
		{
		}

		inline int32_t compare(const FunctionStackItem& functionStackItem) const
		{
			if (this->parenthesisLevel != functionStackItem.parenthesisLevel)
			{
				return this->parenthesisLevel - functionStackItem.parenthesisLevel;
			}
			return this->precedence - functionStackItem.precedence;
		}

		inline bool operator<(const FunctionStackItem& other) const 
		{
			return compare(other) < 0;
		}

		inline bool operator<=(const FunctionStackItem& other) const 
		{
			return compare(other) <= 0;
		}

		inline bool operator==(const FunctionStackItem& other) const 
		{
			return compare(other) == 0;
		}

		inline bool operator>=(const FunctionStackItem& other) const 
		{
			return compare(other) >= 0;
		}

		inline bool operator>(const FunctionStackItem& other) const 
		{
			return compare(other) > 0;
		}
	};

	const int DEFAULT_FUNCTION_LIST_COUNT = 12;

	// Sets up the functions that should be usable in the language
	static uint32_t setupFunctions(const char** functionNameList, Function* functionList, uint32_t capacity)
	{
		RIO_ASSERT(capacity >= DEFAULT_FUNCTION_LIST_COUNT, "Not enough space for default functions");
		
		functionNameList[0] = ","; 
		functionList[0] = Function(OP_NOP, 1, 0);

		functionNameList[1] = "+"; 
		functionList[1] = Function(OP_ADD, 12, 2);

		functionNameList[2] = "-"; 
		functionList[2] = Function(OP_SUB, 12, 2);

		functionNameList[3] = "*"; 
		functionList[3] = Function(OP_MUL, 13, 2);

		functionNameList[4] = "/"; 
		functionList[4] = Function(OP_DIV, 13, 2);

		functionNameList[5] = "u-"; 
		functionList[5] = Function(OP_UNARY_MINUS, 16, 1);

		functionNameList[6] = "u+"; 
		functionList[6] = Function(OP_NOP, 16, 0);

		functionNameList[7] = "sin"; 
		functionList[7] = Function(OP_SIN, 17, 1);

		functionNameList[8] = "cos"; 
		functionList[8] = Function(OP_COS, 17, 1);

		functionNameList[9] = "abs"; 
		functionList[9] = Function(OP_ABS, 17, 1);

		functionNameList[10] = "match"; 
		functionList[10] = Function(OP_MATCH, 17, 2);

		functionNameList[11] = "match2d"; 
		functionList[11] = Function(OP_MATCH2D, 17, 4);

		return DEFAULT_FUNCTION_LIST_COUNT;
	}

	uint32_t compile(const char* source
		, uint32_t variableListCount
		, const char** variableNameList
		, uint32_t constantListCount
		, const char** constantNameList
		, const float* constantValueList
		, uint32_t* byteCode
		, uint32_t capacity
	)
	{
		const char* functionNameList[DEFAULT_FUNCTION_LIST_COUNT];
		Function functionList[DEFAULT_FUNCTION_LIST_COUNT];
		uint32_t functionListCount = setupFunctions(functionNameList, functionList, DEFAULT_FUNCTION_LIST_COUNT);

		CompileEnvironment compileEnvironment;
		compileEnvironment.variableListCount = variableListCount;
		compileEnvironment.variableNameList = variableNameList;
		compileEnvironment.constantListCount = constantListCount;
		compileEnvironment.constantNameList = constantNameList;
		compileEnvironment.constantValueList = constantValueList;
		compileEnvironment.functionListCount = functionListCount;
		compileEnvironment.functionNameList = functionNameList;
		compileEnvironment.functionValueList = functionList;

		uint32_t tokenListCount = tokenize(source, compileEnvironment, nullptr, 0);

		// Change alloca to some other temp memory allocator if you want to
		// TODO memory allocation
		Token* tokenList = (Token*)alloca(sizeof(Token) * tokenListCount);
		tokenize(source, compileEnvironment, tokenList, tokenListCount);

		Token* reversePolishTokenList = (Token*)alloca(sizeof(Token) * tokenListCount);
		uint32_t reversePolishTokenListCount = 0;

		// TODO memory allocation
		FunctionStackItem* functionStackItemList = (FunctionStackItem *)alloca(sizeof(FunctionStackItem) * tokenListCount);
		uint32_t functionStackItemListCount = 0;

		int32_t parenthesisLevel = 0;
		for (uint32_t i = 0; i < tokenListCount; ++i)
		{
			Token& token = tokenList[i];
			switch (token.type) 
			{
				case Token::NUMBER:
				case Token::VARIABLE:
					reversePolishTokenList[reversePolishTokenListCount++] = token;
					break;
				case Token::LEFT_PARENTHESIS:
					++parenthesisLevel;
					break;
				case Token::RIGHT_PARENTHESIS:
					--parenthesisLevel;
					break;
				case Token::FUNCTION: 
				{
					FunctionStackItem functionStackItem(token, compileEnvironment.functionValueList[token.id].precedence, parenthesisLevel);
					while (functionStackItemListCount > 0 && functionStackItemList[functionStackItemListCount - 1] >= functionStackItem)
					{
						reversePolishTokenList[reversePolishTokenListCount++] = functionStackItemList[--functionStackItemListCount].token;
					}
					functionStackItemList[functionStackItemListCount++] = functionStackItem;
					break;
				}
				default:
					RIO_FATAL("Unknown token");
					break;
			}
		}

		while (functionStackItemListCount > 0)
		{
			reversePolishTokenList[reversePolishTokenListCount++] = functionStackItemList[--functionStackItemListCount].token;
		}

		foldConstants(reversePolishTokenList, reversePolishTokenListCount, compileEnvironment);

		return generateByteCode(reversePolishTokenList, reversePolishTokenListCount, compileEnvironment, byteCode, capacity);
	}

	#endif // CAN_COMPILE

	bool getIsConstant(const uint32_t* byteCode)
	{
		uint32_t byteCodeValueCombined = *byteCode++;
		if (!getIsByteCodePushFloat(byteCodeValueCombined))
		{
			return false;
		}
		byteCodeValueCombined = *byteCode;
		return getByteCodeOperation(byteCodeValueCombined) == BYTE_CODE_END;
	}

	float getConstantValue(const uint32_t* byteCode)
	{
		uint32_t byteCodeValueCombined = *byteCode;
		if (getIsByteCodePushFloat(byteCodeValueCombined))
		{
			return getFloatFromUint32(byteCodeValueCombined);
		}
		else 
		{
			RIO_FATAL("Not a static expression");
			return 0;
		}
	}

	bool run(const uint32_t* byteCode, const float* variableList, Stack& stack)
	{
		const uint32_t* byteCodePoinerCurrent = byteCode;
		while (true) 
		{
			uint32_t byteCodeValueCombined = *byteCodePoinerCurrent++;
			uint32_t byteCodeOperation = getByteCodeOperation(byteCodeValueCombined);
			uint32_t id = getByteCodeIdPart(byteCodeValueCombined);
			switch (byteCodeOperation)
			{
				case BYTE_CODE_PUSH_VARIABLE:
					if (stack.size == stack.capacity)
					{
						return false;
					}
					stack.data[stack.size++] = variableList[id];
					break;
				case BYTE_CODE_FUNCTION:
					computeFunction((OpCode)id, stack);
					break;
				case BYTE_CODE_END:
					return true;
				default: // BYTE_CODE_PUSH_FLOAT
					if (stack.size == stack.capacity)
					{
						return false;
					}
					stack.data[stack.size++] = getFloatFromUint32(byteCodeValueCombined);
					break;

			}
		}
	}

} // namespace ExpressionLanguageFn

} // namespace Rio
