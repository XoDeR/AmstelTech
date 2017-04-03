// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/FileSystem/File.h"
#include "Core/Base/RioCommon.h"
#include "Core/Math/Uint32T.h"

#include "Core/Memory/Allocator.h"

#include "Core/Error/Error.h"

#include <malloc.h> // instead of #include <alloca.h> for Visual Studio 2015 https://msdn.microsoft.com/en-us/library/wb1s57t5.aspx
#include <stdarg.h> // va_list
#include <stdio.h>
#include <string.h>

#if RIO_CRT_MSVC
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#elif RIO_PLATFORM_ANDROID || RIO_PLATFORM_BSD || RIO_PLATFORM_IOS || RIO_PLATFORM_OSX || RIO_PLATFORM_QNX
#define fseeko64 fseeko
#define ftello64 ftello
#endif // RIO_

RIO_ERROR_RESULT(RIO_ERROR_READERWRITER_OPEN, RIO_MAKEFOURCC('R', 'W', 0, 1));
RIO_ERROR_RESULT(RIO_ERROR_READERWRITER_READ, RIO_MAKEFOURCC('R', 'W', 0, 2));
RIO_ERROR_RESULT(RIO_ERROR_READERWRITER_WRITE, RIO_MAKEFOURCC('R', 'W', 0, 3));

namespace RioCore
{
    class BinaryWriter
    {
	private:
		File& file;
    public:
        BinaryWriter(File& file)
            : file(file)
        {
        }

        void write(const void* data, uint32_t size)
        {
            file.write(data, size);
        }

        template <typename T>
        void write(const T& data)
        {
            file.write(&data, sizeof(T));
        }

        void skip(uint32_t bytes)
        {
            file.skip(bytes);
        }
    };

    class BinaryReader
    {
	public:
		File& file;
    public:
        BinaryReader(File& file)
            : file(file)
        {
        }

        void read(void* data, uint32_t size)
        {
            file.read(data, size);
        }

        template <typename T>
        void read(T& data)
        {
            file.read(&data, sizeof(T));
        }

        void skip(uint32_t bytes)
        {
            file.skip(bytes);
        }
    };

    struct Whence
    {
        enum Enum
        {
            Begin,
            Current,
            End,
        };
    };

    struct RIO_NO_VTABLE ReaderI
    {
        virtual ~ReaderI() = 0;
        virtual int32_t read(void* _data, int32_t _size, Error* _err) = 0;
    };

    inline ReaderI::~ReaderI()
    {
    }

    struct RIO_NO_VTABLE WriterI
    {
        virtual ~WriterI() = 0;
        virtual int32_t write(const void* data, int32_t size, Error* err) = 0;
    };

    inline WriterI::~WriterI()
    {
    }

    struct RIO_NO_VTABLE SeekerI
    {
        virtual ~SeekerI() = 0;
        virtual int64_t seek(int64_t offset = 0, Whence::Enum whence = Whence::Current) = 0;
    };

    inline SeekerI::~SeekerI()
    {
    }

    // Read data
    inline int32_t read(ReaderI* reader, void* data, int32_t size, Error* err = nullptr)
    {
        RIO_ERROR_SCOPE(err);
        return reader->read(data, size, err);
    }

    // Read value
    template<typename Ty>
    inline int32_t read(ReaderI* reader, Ty& value, Error* err = nullptr)
    {
        RIO_ERROR_SCOPE(err);
        RIO_STATIC_ASSERT(RIO_TYPE_IS_POD(Ty));
        return reader->read(&value, sizeof(Ty), err);
    }

    // Read value and converts it to host endianess. fromLittleEndian specifies
    // underlying stream endianess
    template<typename Ty>
    inline int32_t readHE(ReaderI* _reader, Ty& _value, bool _fromLittleEndian, Error* _err = nullptr)
    {
        RIO_ERROR_SCOPE(_err);
        RIO_STATIC_ASSERT(RIO_TYPE_IS_POD(Ty));
        Ty value;
        int32_t result = _reader->read(&value, sizeof(Ty), _err);
        _value = toHostEndian(value, _fromLittleEndian);
        return result;
    }

    // Write data.
    inline int32_t write(WriterI* _writer, const void* _data, int32_t _size, Error* _err = nullptr)
    {
        RIO_ERROR_SCOPE(_err);
        return _writer->write(_data, _size, _err);
    }

    // Write repeat the same value.
    inline int32_t writeRep(WriterI* _writer, uint8_t _byte, int32_t _size, Error* _err = nullptr)
    {
        RIO_ERROR_SCOPE(_err);

        const uint32_t tmp0 = uint32_sels(64 - _size, 64, _size);
        const uint32_t tmp1 = uint32_sels(256 - _size, 256, tmp0);
        const uint32_t blockSize = uint32_sels(1024 - _size, 1024, tmp1);
        uint8_t* temp = (uint8_t*)alloca(blockSize);
        memset(temp, _byte, blockSize);

        int32_t size = 0;
        while (0 < _size)
        {
            int32_t bytes = write(_writer, temp, uint32_min(blockSize, _size), _err);
            size += bytes;
            _size -= bytes;
        }

        return size;
    }

    // Write value
    template<typename Ty>
    inline int32_t write(WriterI* _writer, const Ty& _value, Error* _err = nullptr)
    {
        RIO_ERROR_SCOPE(_err);
        RIO_STATIC_ASSERT(RIO_TYPE_IS_POD(Ty));
        return _writer->write(&_value, sizeof(Ty), _err);
    }

    // Write value as little endian
    template<typename Ty>
    inline int32_t writeLE(WriterI* _writer, const Ty& _value, Error* _err = nullptr)
    {
        RIO_ERROR_SCOPE(_err);
        RIO_STATIC_ASSERT(RIO_TYPE_IS_POD(Ty));
        Ty value = toLittleEndian(_value);
        int32_t result = _writer->write(&value, sizeof(Ty), _err);
        return result;
    }

    // Write value as big endian
    template<typename Ty>
    inline int32_t writeBE(WriterI* writer, const Ty& value, Error* err = nullptr)
    {
        RIO_ERROR_SCOPE(err);
        RIO_STATIC_ASSERT(RIO_TYPE_IS_POD(Ty));
        Ty valueLocal = toBigEndian(value);
        int32_t result = writer->write(&valueLocal, sizeof(Ty), err);
        return result;
    }

    // Write formated string
    inline int32_t writePrintf(WriterI* writer, const char* format, ...)
    {
        va_list argList;
        va_start(argList, format);

        char temp[2048];
        char* out = temp;
        int32_t max = sizeof(temp);
        int32_t len = vsnprintf(out, max, format, argList);
        if (len > max)
        {
            out = (char*)alloca(len);
            len = vsnprintf(out, len, format, argList);
        }

        int32_t size = write(writer, out, len);

        va_end(argList);

        return size;
    }

    // Skip _offset bytes forward
    inline int64_t skip(SeekerI* seeker, int64_t offset)
    {
        return seeker->seek(offset, Whence::Current);
    }

    // Seek to any position in file
    inline int64_t seek(SeekerI* seeker, int64_t offset = 0, Whence::Enum whence = Whence::Current)
    {
        return seeker->seek(offset, whence);
    }

    // Returns size of file
    inline int64_t getSize(SeekerI* seeker)
    {
        int64_t offset = seeker->seek();
        int64_t size = seeker->seek(0, Whence::End);
        seeker->seek(offset, Whence::Begin);
        return size;
    }

    struct RIO_NO_VTABLE ReaderSeekerI : public ReaderI, public SeekerI
    {
    };

    // Peek data
    inline int32_t peek(ReaderSeekerI* reader, void* data, int32_t size, Error* err = nullptr)
    {
        RIO_ERROR_SCOPE(err);
        int64_t offset = RioCore::seek(reader);
        int32_t size = reader->read(data, size, err);
        RioCore::seek(reader, offset, RioCore::Whence::Begin);
        return size;
    }

    // Peek value
    template<typename Ty>
    inline int32_t peek(ReaderSeekerI* reader, Ty& value, Error* err = nullptr)
    {
        RIO_ERROR_SCOPE(err);
        RIO_STATIC_ASSERT(RIO_TYPE_IS_POD(Ty));
        return peek(reader, &value, sizeof(Ty), err);
    }

    struct RIO_NO_VTABLE WriterSeekerI : public WriterI, public SeekerI
    {
    };

    struct RIO_NO_VTABLE ReaderOpenI
    {
        virtual ~ReaderOpenI() = 0;
        virtual bool open(const char* filePath, Error* err) = 0;
    };

    inline ReaderOpenI::~ReaderOpenI()
    {
    }

    struct RIO_NO_VTABLE WriterOpenI
    {
        virtual ~WriterOpenI() = 0;
        virtual bool open(const char* filePath, bool append, Error* err) = 0;
    };

    inline WriterOpenI::~WriterOpenI()
    {
    }

    struct RIO_NO_VTABLE CloserI
    {
        virtual ~CloserI() = 0;
        virtual void close() = 0;
    };

    inline CloserI::~CloserI()
    {
    }

    struct RIO_NO_VTABLE FileReaderI : public ReaderOpenI, public CloserI, public ReaderSeekerI
    {
    };

    struct RIO_NO_VTABLE FileWriterI : public WriterOpenI, public CloserI, public WriterSeekerI
    {
    };

    inline bool open(ReaderOpenI* _reader, const char* filePath, Error* err = nullptr)
    {
        RIO_ERROR_USE_TEMP_WHEN_NULL(err);
        return _reader->open(filePath, err);
    }

    inline bool open(WriterOpenI* writer, const char* filePath, bool append = false, Error* err = nullptr)
    {
        RIO_ERROR_USE_TEMP_WHEN_NULL(err);
        return writer->open(filePath, append, err);
    }

    inline void close(CloserI* reader)
    {
        reader->close();
    }

    struct RIO_NO_VTABLE MemoryBlockI
    {
        virtual void* more(uint32_t size = 0) = 0;
        virtual uint32_t getSize() = 0;
    };

    class StaticMemoryBlock : public MemoryBlockI
    {
	private:
		void* m_data;
		uint32_t m_size;
    public:
        StaticMemoryBlock(void* data, uint32_t size)
            : m_data(data)
            , m_size(size)
        {
        }

        virtual ~StaticMemoryBlock()
        {
        }

        virtual void* more(uint32_t /*_size*/ = 0) RIO_OVERRIDE
        {
            return m_data;
        }

        virtual uint32_t getSize() RIO_OVERRIDE
        {
            return m_size;
        }
    };

    class MemoryBlock : public MemoryBlockI
    {
	private:
		AllocatorI* m_allocator;
		void* m_data = nullptr;
		uint32_t m_size = 0;
    public:
        MemoryBlock(AllocatorI* allocator)
            : m_allocator(allocator)
        {
        }

        virtual ~MemoryBlock()
        {
            RIO_FREE(this->m_allocator, this->m_data);
        }

        virtual void* more(uint32_t size = 0) RIO_OVERRIDE
        {
            if (0 < size)
            {
                this->m_size += size;
                this->m_data = RIO_REALLOC(m_allocator, this->m_data, this->m_size);
            }

            return this->m_data;
        }

        virtual uint32_t getSize() RIO_OVERRIDE
        {
            return m_size;
        }
    };

    class SizerWriter : public WriterSeekerI
    {
	private:
		int64_t m_pos = 0;
		int64_t m_top = 0;
    public:
        SizerWriter()
        {
        }

        virtual ~SizerWriter()
        {
        }

        virtual int64_t seek(int64_t offset = 0, Whence::Enum whence = Whence::Current) RIO_OVERRIDE
        {
            switch (_whence)
            {
            case Whence::Begin:
                m_pos = Int64T::clamp(offset, 0, m_top);
                break;

            case Whence::Current:
                m_pos = Int64T::clamp(m_pos + offset, 0, m_top);
                break;

            case Whence::End:
                m_pos = Int64T::clamp(m_top - offset, 0, m_top);
                break;
            }

            return m_pos;
        }

        virtual int32_t write(const void* /*_data*/, int32_t size, Error* err) RIO_OVERRIDE
        {
            RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

            int32_t morecore = int32_t(m_pos - m_top) + size;

            if (0 < morecore)
            {
                m_top += morecore;
            }

            int64_t remainder = m_top - m_pos;
            int32_t sizeLocal = uint32_min(size, uint32_t(int64_min(remainder, INT32_MAX)));
            m_pos += sizeLocal;
            if (sizeLocal != size)
            {
                RIO_ERROR_SET(err, RIO_ERROR_READERWRITER_WRITE, "SizerWriter: write truncated.");
            }
            return sizeLocal;
        }
    };

    class MemoryReader : public ReaderSeekerI
    {
	private:
		const uint8_t* m_data;
		int64_t m_pos = 0;
		int64_t m_top;
    public:
        MemoryReader(const void* data, uint32_t size)
            : m_data((const uint8_t*)data)
            , m_top(size)
        {
        }

        virtual ~MemoryReader()
        {
        }

        virtual int64_t seek(int64_t offset, Whence::Enum whence) RIO_OVERRIDE
        {
            switch (whence)
            {
            case Whence::Begin:
                m_pos = Int64T::clamp(offset, 0, m_top);
                break;

            case Whence::Current:
                m_pos = Int64T::clamp(m_pos + offset, 0, m_top);
                break;

            case Whence::End:
                m_pos = Int64T::clamp(m_top - offset, 0, m_top);
                break;
            }

            return m_pos;
        }

        virtual int32_t read(void* _data, int32_t _size, Error* _err) RIO_OVERRIDE
        {
            RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

            int64_t remainder = m_top - m_pos;
            int32_t size = uint32_min(_size, uint32_t(int64_min(remainder, INT32_MAX)));
            memcpy(_data, &m_data[m_pos], size);
            m_pos += size;
            if (size != _size)
            {
                RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_READ, "MemoryReader: read truncated.");
            }
            return size;
        }

        const uint8_t* getDataPtr() const
        {
            return &m_data[m_pos];
        }

        int64_t getPos() const
        {
            return m_pos;
        }

        int64_t remaining() const
        {
            return m_top - m_pos;
        }
    };

    class MemoryWriter : public WriterSeekerI
    {
	private:
		MemoryBlockI* m_memBlock;
		uint8_t* m_data = nullptr;
		int64_t m_pos = 0;
		int64_t m_top = 0;
		int64_t m_size = 0;
    public:
        MemoryWriter(MemoryBlockI* memBlock)
            : m_memBlock(memBlock)
        {
        }

        virtual ~MemoryWriter()
        {
        }

        virtual int64_t seek(int64_t offset = 0, Whence::Enum whence = Whence::Current) RIO_OVERRIDE
        {
            switch (whence)
            {
            case Whence::Begin:
                m_pos = Int64T::clamp(offset, 0, m_top);
                break;

            case Whence::Current:
                m_pos = Int64T::clamp(m_pos + offset, 0, m_top);
                break;

            case Whence::End:
                m_pos = Int64T::clamp(m_top - offset, 0, m_top);
                break;
            }

            return m_pos;
        }

        virtual int32_t write(const void* _data, int32_t _size, Error* _err) RIO_OVERRIDE
        {
            RIO_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

            int32_t morecore = int32_t(m_pos - m_size) + _size;

            if (0 < morecore)
            {
                morecore = RIO_ALIGN_MASK(morecore, 0xfff);
                m_data = (uint8_t*)m_memBlock->more(morecore);
                m_size = m_memBlock->getSize();
            }

            int64_t remainder = m_size - m_pos;
            int32_t size = uint32_min(_size, uint32_t(int64_min(remainder, INT32_MAX)));
            memcpy(&m_data[m_pos], _data, size);
            m_pos += size;
            m_top = int64_max(m_top, m_pos);
            if (size != _size)
            {
                RIO_ERROR_SET(_err, RIO_ERROR_READERWRITER_WRITE, "MemoryWriter: write truncated.");
            }
            return size;
        }
    };

    class StaticMemoryBlockWriter : public MemoryWriter
    {
	private:
		StaticMemoryBlock staticMemoryBlock;
    public:
        StaticMemoryBlockWriter(void* data, uint32_t size)
            : MemoryWriter(&staticMemoryBlock)
            , staticMemoryBlock(data, size)
        {
        }

        ~StaticMemoryBlockWriter()
        {
        }
    };
} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka