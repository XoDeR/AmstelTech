#pragma once

#include <cstdint>


namespace Rio
{

namespace ExpressionLanguageFn
{
	// A small language for computing arithmetic expressions, such as (a + b)*3 - sin(y)
	// It allows you freedom in specifying variable dependencies, while avoiding the complexities of a full blown scripting language
	//
	// The language allows for constants (which are converted to numbers at compile time) and variables (which are evaluated as numbers during run time)
	// The language has a fixed small set of operators and functions (more are easy to add)
	// All constants, variables and stack values used by the language are 32 bit floats
	//
	// During the compilation phase, the source string is compiled to byte code
	// which is run during the run phase. The virtual machine is a simple stack-based machine with the stack explicitly supplied by the caller
	//
	// All instructions in the byte code are 32-bit wide
	// The instruction is either a regular float in which case the operation is BYTE_CODE_PUSH_FLOAT -- the float is pushed to the stack, or it is a NaN
	// If the float is a NaN, the mantissa is used to encode the type of operation and additional data
	// The first three bits encode the operation and the remaining 20 bits encode the operation data
	//
	// NAN_MARKER (9) BYTE_CODE_PUSH_VARIABLE (3) id (20) Pushes the variable with the specified id
	// NAN_MARKER (9) BYTE_CODE_FUNCTION (3) id (20) Computes the function with the specified id
	// NAN_MARKER (9) BYTE_CODE_END (3) zero (20) Marks the end of the byte code
	//
	// float (32) Pushes the float

// CAN_COMPILE --a flag used to include the parts of the code needed to compile to bytecode
// If you compile offline you can exclude this code in the runtime version
#define CAN_COMPILE
#ifdef CAN_COMPILE
	
	// Compiles the <source> and stores the result in the <byteCode> 
	// <variableNameList> is a list of variable names
	// The position of the variable in the list should match the position when <variableNameList> is sent to the run function
	//
	// <constantNameList> and <constantValueList> specifies a list of runtime constants and corresponding values
	// Constants are expanded to numbers at compile time
	//
	// Returns the number of compiled unsigned words
	// If the returned number is greater than <byteCodeCapacity>, 
	// only the first <byteCodeCapacity> words of the byte code are written to <byteCode>
	uint32_t compile(const char* source
		, uint32_t variableListCount
		, const char** variableNameList
		, uint32_t constantListCount
		, const char** constantNameList
		, const float* constantValueList
		, uint32_t* byteCode
		, uint32_t byteCodeCapacity
	);

#endif // CAN_COMPILE

	// Returns true if the byte code is constant
	// i. e. if it always produces the same float value
	bool getIsConstant(const uint32_t* byteCode);

	// Returns the constant value produced by the byte code
	float getConstantValue(const uint32_t* byteCode);

	// Represents the working stack
	struct Stack
	{
		float* data = nullptr;
		uint32_t size = 0;
		uint32_t capacity = 0;

		Stack(float* data, uint32_t capacity)
			: data(data)
			, capacity(capacity) 
		{
		}
	};

	// Runs the <byteCode> using the <stack> as execution stack
	// <variableList> is a list of variable values to use for the execution
	// They should match the list of variable names supplied to the compile function
	bool run(const uint32_t* byteCode, const float* variableList, Stack& stack);

} // namespace ExpressionLanguageFn

} // namespace Rio
