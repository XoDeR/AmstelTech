// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"
#include "Core/Base/Cpu.h"
#include "Core/Math/Uint32T.h"

namespace RioCore
{
	class RingBufferControl
	{
	public:
		const uint32_t m_size;
		uint32_t m_current = 0;
		uint32_t m_write = 0;
		uint32_t m_read = 0;
	private:
		RingBufferControl(const RingBufferControl& ringBufferControl) = delete;
		RingBufferControl& operator=(const RingBufferControl& ringBufferControl) = delete;
	public:
		explicit RingBufferControl(uint32_t size)
			: m_size(size)
		{
		}

		~RingBufferControl()
		{
		}

		uint32_t available() const
		{
			return distance(m_read, m_current);
		}

		uint32_t consume(uint32_t _size) // consumer only
		{
			const uint32_t maxSize = distance(m_read, m_current);
			const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
			const uint32_t test = uint32_sub(sizeNoSign, maxSize);
			const uint32_t sizeLocal = uint32_sels(test, _size, maxSize);
			const uint32_t advance = uint32_add(m_read, sizeLocal);
			const uint32_t read = uint32_mod(advance, m_size);
			m_read = read;
			return sizeLocal;
		}

		uint32_t reserve(uint32_t _size) // producer only
		{
			const uint32_t dist = distance(m_write, m_read) - 1;
			const uint32_t maxSize = uint32_sels(dist, m_size - 1, dist);
			const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
			const uint32_t test = uint32_sub(sizeNoSign, maxSize);
			const uint32_t sizeLocal = uint32_sels(test, _size, maxSize);
			const uint32_t advance = uint32_add(m_write, sizeLocal);
			const uint32_t write = uint32_mod(advance, m_size);
			m_write = write;
			return sizeLocal;
		}

		uint32_t commit(uint32_t _size) // producer only
		{
			const uint32_t maxSize = distance(m_current, m_write);
			const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
			const uint32_t test = uint32_sub(sizeNoSign, maxSize);
			const uint32_t sizeLocal = uint32_sels(test, _size, maxSize);
			const uint32_t advance = uint32_add(m_current, sizeLocal);
			const uint32_t current = uint32_mod(advance, m_size);
			m_current = current;
			return sizeLocal;
		}

		uint32_t distance(uint32_t _from, uint32_t _to) const // both
		{
			const uint32_t diff = uint32_sub(_to, _from);
			const uint32_t le = uint32_add(m_size, diff);
			const uint32_t result = uint32_sels(diff, le, diff);

			return result;
		}

		void reset()
		{
			m_current = 0;
			m_write = 0;
			m_read = 0;
		}
	};

	class SpScRingBufferControl
	{
	public:
		const uint32_t m_size;
		uint32_t m_current = 0;
		uint32_t m_write = 0;
		uint32_t m_read = 0;
	private:
		SpScRingBufferControl(const SpScRingBufferControl& rhs) = delete;
		SpScRingBufferControl& operator=(const SpScRingBufferControl& rhs) = delete;
	public:
		explicit SpScRingBufferControl(uint32_t size)
			: m_size(size)
		{
		}

		~SpScRingBufferControl()
		{
		}

		uint32_t available() const
		{
			return distance(m_read, m_current);
		}

		uint32_t consume(uint32_t _size) // consumer only
		{
			const uint32_t maxSize = distance(m_read, m_current);
			const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
			const uint32_t test = uint32_sub(sizeNoSign, maxSize);
			const uint32_t size = uint32_sels(test, _size, maxSize);
			const uint32_t advance = uint32_add(m_read, size);
			const uint32_t read = uint32_mod(advance, m_size);
			m_read = read;
			return size;
		}

		uint32_t reserve(uint32_t _size) // producer only
		{
			const uint32_t dist = distance(m_write, m_read) - 1;
			const uint32_t maxSize = uint32_sels(dist, m_size - 1, dist);
			const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
			const uint32_t test = uint32_sub(sizeNoSign, maxSize);
			const uint32_t size = uint32_sels(test, _size, maxSize);
			const uint32_t advance = uint32_add(m_write, size);
			const uint32_t write = uint32_mod(advance, m_size);
			m_write = write;
			return size;
		}

		uint32_t commit(uint32_t _size) // producer only
		{
			const uint32_t maxSize = distance(m_current, m_write);
			const uint32_t sizeNoSign = uint32_and(_size, 0x7fffffff);
			const uint32_t test = uint32_sub(sizeNoSign, maxSize);
			const uint32_t size = uint32_sels(test, _size, maxSize);
			const uint32_t advance = uint32_add(m_current, size);
			const uint32_t current = uint32_mod(advance, m_size);

			// must commit all memory writes before moving m_current pointer
			// once m_current pointer moves data is used by consumer thread
			memoryBarrier();
			m_current = current;
			return size;
		}

		uint32_t distance(uint32_t _from, uint32_t _to) const // both
		{
			const uint32_t diff = uint32_sub(_to, _from);
			const uint32_t le = uint32_add(m_size, diff);
			const uint32_t result = uint32_sels(diff, le, diff);

			return result;
		}

		void reset()
		{
			m_current = 0;
			m_write = 0;
			m_read = 0;
		}
	};

	template <typename Control>
	class ReadRingBufferT
	{
	private:
		template <typename Ty>
		friend class WriteRingBufferT;

		Control& m_control;
		uint32_t m_read;
		uint32_t m_end;
		const uint32_t m_size;
		const char* m_buffer;
	private:
		ReadRingBufferT(const ReadRingBufferT& rhs) = delete;
		ReadRingBufferT& operator=(const ReadRingBufferT& rhs) = delete;
		ReadRingBufferT() = delete;
	public:
		ReadRingBufferT(Control& control, const char* buffer, uint32_t size)
			: m_control(control)
			, m_read(control.m_read)
			, m_end(m_read + size)
			, m_size(size)
			, m_buffer(buffer)
		{
			RIO_CHECK(control.available() >= _size, "%d >= %d", control.available(), size);
		}

		~ReadRingBufferT()
		{
		}

		void end()
		{
			m_control.consume(m_size);
		}

		void read(char* _data, uint32_t _len)
		{
			const uint32_t eof = (m_read + _len) % m_control.m_size;
			uint32_t wrap = 0;
			const char* from = &m_buffer[m_read];

			if (eof < m_read)
			{
				wrap = m_control.m_size - m_read;
				memcpy(_data, from, wrap);
				_data += wrap;
				from = (const char*)&m_buffer[0];
			}

			memcpy(_data, from, _len - wrap);

			m_read = eof;
		}

		void skip(uint32_t _len)
		{
			m_read += _len;
			m_read %= m_control.m_size;
		}
	};

	using ReadRingBuffer = ReadRingBufferT<RingBufferControl>;
	using SpScReadRingBuffer = ReadRingBufferT<SpScRingBufferControl>;

	template <typename Control>
	class WriteRingBufferT
	{
	private:
		Control& m_control;
		uint32_t m_write;
		uint32_t m_end;
		const uint32_t m_size;
		char* m_buffer;
	private:
		WriteRingBufferT(const WriteRingBufferT& rhs) = delete;
		WriteRingBufferT& operator=(const WriteRingBufferT& rhs) = delete;
		WriteRingBufferT() = delete;
	public:
		WriteRingBufferT(Control& _control, char* _buffer, uint32_t _size)
			: m_control(_control)
			, m_size(_size)
			, m_buffer(_buffer)
		{
			uint32_t size = m_control.reserve(_size);
			RIO_UNUSED(size);
			RIO_CHECK(size == _size, "%d == %d", size, _size);
			m_write = m_control.m_current;
			m_end = m_write + _size;
		}

		~WriteRingBufferT()
		{
		}

		void end()
		{
			m_control.commit(m_size);
		}

		void write(const char* _data, uint32_t length)
		{
			const uint32_t eof = (m_write + length) % m_control.m_size;
			uint32_t wrap = 0;
			char* to = &m_buffer[m_write];

			if (eof < m_write)
			{
				wrap = m_control.m_size - m_write;
				memcpy(to, _data, wrap);
				_data += wrap;
				to = (char*)&m_buffer[0];
			}

			memcpy(to, _data, length - wrap);

			m_write = eof;
		}

		void write(ReadRingBufferT<Control>& _read, uint32_t _len)
		{
			const uint32_t eof = (_read.m_read + _len) % _read.m_control.m_size;
			uint32_t wrap = 0;
			const char* from = &_read.m_buffer[_read.m_read];

			if (eof < _read.m_read)
			{
				wrap = _read.m_control.m_size - _read.m_read;
				write(from, wrap);
				from = (const char*)&_read.m_buffer[0];
			}

			write(from, _len - wrap);

			_read.m_read = eof;
		}

		void skip(uint32_t _len)
		{
			m_write += _len;
			m_write %= m_control.m_size;
		}
	};

	using WriteRingBuffer = WriteRingBufferT<RingBufferControl>;
	using SpScWriteRingBuffer = WriteRingBufferT<SpScRingBufferControl>;

} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka