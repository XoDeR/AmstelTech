// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include <stdarg.h> // va_list
#include <stdint.h>

#include "Config.h"
#include "Core/Base/RioCommon.h"

#define RIO_ERROR_SET(ptr, result, msg) \
			RIO_MACRO_BLOCK_BEGIN \
				RIO_TRACE("Error %d: %s", result.code, "" msg); \
				ptr->setError(result,  "" msg); \
			RIO_MACRO_BLOCK_END

#define RIO_ERROR_USE_TEMP_WHEN_NULL(ptr) \
			const RioCore::Error tmpError; /* It should not be used directly */ \
			ptr = NULL == ptr ? const_cast<RioCore::Error*>(&tmpError) : ptr

#define RIO_ERROR_SCOPE(ptr) \
			RIO_ERROR_USE_TEMP_WHEN_NULL(ptr); \
			RioCore::ErrorScope rioErrorScope(const_cast<RioCore::Error*>(&tmpError) )

#define RIO_ERROR_RESULT(err, code) \
			RIO_STATIC_ASSERT(code != 0, "ErrorCode 0 is reserved!"); \
			static const RioCore::ErrorResult err = { code }

namespace RioCore
{

    struct ErrorResult
    {
        uint32_t code;
    };

    class Error
    {
    private:
        const char* m_msg = nullptr;
        uint32_t m_code = 0;
    private:
        Error(const Error& rhs) = delete;
        Error& operator=(const Error& rhs) = delete;
    public:
        Error()
        {
        }

        void setError(ErrorResult errorResult, const char* msg)
        {
            RIO_CHECK(0 != errorResult.code, "Invalid ErrorResult passed to setError!");

            if (!isOk())
            {
                return;
            }

            m_code = errorResult.code;
            m_msg = msg;
        }

        bool isOk() const
        {
            return 0 == m_code;
        }

        ErrorResult get() const
        {
            ErrorResult result = { m_code };
            return result;
        }

        bool operator==(ErrorResult errorResultOther) const
        {
            return errorResultOther.code == this->m_code;
        }
    };

    class ErrorScope
    {
    private:
        Error* error = nullptr;
    private:
        ErrorScope(const ErrorScope& rhs) = delete;
        ErrorScope& operator=(const ErrorScope& rhs) = delete;
    public:
        ErrorScope(Error* error)
            : error(error)
        {
            RIO_CHECK(NULL != error, "error can't be NULL");
        }

        ~ErrorScope()
        {
            RIO_CHECK(error->isOk(), "Error: %d", error->get().code);
        }
    };


    namespace ErrorFn
    {
        // Aborts the program execution logging an error message and the stacktrace if the platform supports it
        void abort(const char* file, int line, const char* format, ...);
        void printCallstack();
    } // namespace ErrorFn

} // namespace RioCore

#if RIO_DEBUG
#define RIO_ASSERT(condition, message, ...) do { if (!(condition)) {\
		RioCore::ErrorFn::abort(__FILE__, __LINE__, "\nAssertion failed: %s\n\t" message "\n", #condition, ##__VA_ARGS__); }} while (0)
#else
#define RIO_ASSERT(...) ((void)0)
#endif // RIO_DEBUG

#define RIO_ASSERT_NOT_NULL(pointer) RIO_ASSERT(pointer != nullptr, #pointer " must be not null")
#define RIO_FATAL(message) RIO_ASSERT(false, message)
#define RIO_ENSURE(condition) RIO_ASSERT(condition, "")

#define DBG_STRINGIZE(_x) DBG_STRINGIZE_(_x)
#define DBG_STRINGIZE_(_x) #_x
#define DBG_FILE_LINE_LITERAL "" __FILE__ "(" DBG_STRINGIZE(__LINE__) "): "
#define DBG(format, ...) dbgPrintf(DBG_FILE_LINE_LITERAL "" format "\n", ##__VA_ARGS__)

extern void dbgPrintfVargs(const char* format, va_list argList);
extern void dbgPrintf(const char* format, ...);

// Copyright (c) 2016, 2017 Volodymyr Syvochka