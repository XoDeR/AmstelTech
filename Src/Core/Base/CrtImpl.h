#pragma once

#if RIO_CONFIG_ALLOCATOR_CRT
#include <malloc.h>

#include "Core/Memory/Allocator.h"
#endif // RIO_CONFIG_ALLOCATOR_CRT

#if RIO_CONFIG_CRT_FILE_READER_WRITER
#include "Core/FileSystem/ReaderWriter.h"
#endif // RIO_CONFIG_CRT_FILE_READER_WRITER

namespace RioCore
{
#if RIO_CONFIG_ALLOCATOR_CRT
	class CrtAllocator : public AllocatorI
	{
	public:
		CrtAllocator()
		{
		}

		virtual ~CrtAllocator()
		{
		}

		virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) RIO_OVERRIDE
		{
			if (0 == _size)
			{
				if (NULL != _ptr)
				{
					if (RIO_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
					{
						::free(_ptr);
						return NULL;
					}

#if RIO_COMPILER_MSVC
					RIO_UNUSED(_file, _line);
					_aligned_free(_ptr);
#else
                    RioCore::alignedFree(this, _ptr, _align, _file, _line);
#endif // RIO_
				}

				return NULL;
			}
			else if (NULL == _ptr)
			{
				if (RIO_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
				{
					return ::malloc(_size);
				}

#if RIO_COMPILER_MSVC
				RIO_UNUSED(_file, _line);
				return _aligned_malloc(_size, _align);
#else
				return RioCore::alignedAlloc(this, _size, _align, _file, _line);
#endif // RIO_
			}

			if (RIO_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
			{
				return ::realloc(_ptr, _size);
			}

#if RIO_COMPILER_MSVC
			RIO_UNUSED(_file, _line);
			return _aligned_realloc(_ptr, _size, _align);
#else
			return RioCore::alignedRealloc(this, _ptr, _size, _align, _file, _line);
#endif // RIO_
		}
	};
#endif // RIO_CONFIG_ALLOCATOR_CRT

#if RIO_CONFIG_CRT_FILE_READER_WRITER
	class CrtFileReader : public FileReaderI
	{
	private:
		FILE* m_file = nullptr;
	public:
		CrtFileReader()
		{
		}

		virtual ~CrtFileReader()
		{
		}

		virtual bool open(const char* _filePath, Error* _err) RIO_OVERRIDE
		{
			RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			m_file = fopen(_filePath, "rb");
			if (NULL == m_file)
			{
				RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_OPEN, "CrtFileReader: Failed to open file.");
				return false;
			}

			return true;
		}

		virtual void close() RIO_OVERRIDE
		{
			fclose(m_file);
		}

		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) RIO_OVERRIDE
		{
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) RIO_OVERRIDE
		{
			RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = (int32_t)fread(_data, 1, _size, m_file);
			if (size != _size)
			{
				RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_READ, "CrtFileReader: read failed.");
				return size >= 0 ? size : 0;
			}

			return size;
		}

	
	};

	class CrtFileWriter : public FileWriterI
	{
	private:
		FILE* m_file;
	public:
		CrtFileWriter()
			: m_file(NULL)
		{
		}

		virtual ~CrtFileWriter()
		{
		}

		virtual bool open(const char* _filePath, bool _append, Error* _err) RIO_OVERRIDE
		{
			m_file = fopen(_filePath, _append ? "ab" : "wb");

			if (NULL == m_file)
			{
				RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_OPEN, "CrtFileWriter: Failed to open file.");
				return false;
			}

			return true;
		}

		virtual void close() RIO_OVERRIDE
		{
			fclose(m_file);
		}

		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) RIO_OVERRIDE
		{
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) RIO_OVERRIDE
		{
			RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = (int32_t)fwrite(_data, 1, _size, m_file);
			if (size != _size)
			{
				RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_WRITE, "CrtFileWriter: write failed.");
				return size >= 0 ? size : 0;
			}

			return size;
		}
	};
#endif // RIO_CONFIG_CRT_FILE_READER_WRITER

#if RIO_CONFIG_CRT_PROCESS

#if RIO_CRT_MSVC
#define popen _popen
#define pclose _pclose
#endif // RIO_CRT_MSVC

	class ProcessReader : public ReaderOpenI, public CloserI, public ReaderI
	{
	private:
		FILE* m_file = nullptr;
	public:
		ProcessReader()
		{
		}

		~ProcessReader()
		{
			RIO_CHECK(NULL == m_file, "Process not closed!");
		}

		virtual bool open(const char* _command, Error* _err) RIO_OVERRIDE
		{
			RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			m_file = popen(_command, "r");
			if (NULL == m_file)
			{
				RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_OPEN, "ProcessReader: Failed to open process.");
				return false;
			}

			return true;
		}

		virtual void close() RIO_OVERRIDE
		{
			RIO_CHECK(NULL != m_file, "Process not open!");
			pclose(m_file);
			m_file = NULL;
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) RIO_OVERRIDE
		{
			RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors."); RIO_UNUSED(_err);

			int32_t size = (int32_t)fread(_data, 1, _size, m_file);
			if (size != _size)
			{
				return size >= 0 ? size : 0;
			}

			return size;
		}
	};

	class ProcessWriter : public WriterOpenI, public CloserI, public WriterI
	{
	private:
		FILE* m_file = nullptr;
	public:
		~ProcessWriter()
		{
			RIO_CHECK(NULL == m_file, "Process not closed!");
		}

		virtual bool open(const char* _command, bool, Error* _err) RIO_OVERRIDE
		{
			RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			m_file = popen(_command, "w");
			if (NULL == m_file)
			{
				RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_OPEN, "ProcessWriter: Failed to open process.");
				return false;
			}

			return true;
		}

		virtual void close() RIO_OVERRIDE
		{
			RIO_CHECK(NULL != m_file, "Process not open!");
			pclose(m_file);
			m_file = NULL;
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) RIO_OVERRIDE
		{
			RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors."); RIO_UNUSED(_err);

			int32_t size = (int32_t)fwrite(_data, 1, _size, m_file);
			if (size != _size)
			{
				return size >= 0 ? size : 0;
			}

			return size;
		}
	};
#endif // RIO_CONFIG_CRT_PROCESS

} // namespace RioCore
