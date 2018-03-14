#pragma once

#include "Core/Network/Types.h"
#include "Core/Platform.h"
#include "Core/Types.h"

#if RIO_PLATFORM_POSIX
	using SOCKET = int;
	#define INVALID_SOCKET (-1)
#elif RIO_PLATFORM_WINDOWS
	#include <winsock2.h>
#endif // RIO_PLATFORM_POSIX || RIO_PLATFORM_WINDOWS

namespace Rio
{

struct ConnectResult
{
	enum
	{
		SUCCESS,
		REFUSED,
		TIMEOUT,
		UNKNOWN
	} error;
};

struct BindResult
{
	enum
	{
		SUCCESS,
		ADDRESS_IN_USE,
		UNKNOWN
	} error;
};

struct AcceptResult
{
	enum
	{
		SUCCESS,
		NO_CONNECTION,
		UNKNOWN
	} error;
};

struct ReadResult
{
	enum
	{
		SUCCESS,
		WOULDBLOCK,
		REMOTE_CLOSED,
		TIMEOUT,
		UNKNOWN
	} error;
	uint32_t bytesRead = 0;
};

struct WriteResult
{
	enum
	{
		SUCCESS,
		WOULDBLOCK,
		REMOTE_CLOSED,
		TIMEOUT,
		UNKNOWN
	} error;
	uint32_t bytesWritten = 0;
};

struct TcpSocket
{
	SOCKET socket = INVALID_SOCKET;

	TcpSocket();

	// Closes the socket
	void close();

	// Connects to the <ip> address and <port> and returns the result
	ConnectResult connect(const IpAddress& ip, uint16_t port);

	// Binds the socket to <port> and returns the result
	BindResult bind(uint16_t port);

	// Listens for <max> socket connections
	void listen(uint32_t max);

	// Accepts a new connection <c>
	AcceptResult accept(TcpSocket& tcpSocket);

	// Accepts a new connection <c>
	AcceptResult acceptNonBlocking(TcpSocket& tcpSocket);

	// Reads <size> bytes and returns the result
	ReadResult read(void* data, uint32_t size);

	// Reads <size> bytes and returns the result
	ReadResult readNonBlocking(void* data, uint32_t size);

	// Writes <size> bytes and returns the result
	WriteResult write(const void* data, uint32_t size);

	// Writes <size> bytes and returns the result
	WriteResult writeNonBlocking(const void* data, uint32_t size);

	// Sets whether the socket is <isBlocking>
	void setIsBlocking(bool isBlocking);

	// Sets whether the socket should <reuse> a busy port
	void setReuseAddress(bool reuse);

	// Sets the timeout to the given <seconds>
	void setTimeout(uint32_t seconds);
};

} // namespace Rio
