#include "Core/Network/Socket.h"

#include "Core/Error/Error.h"
#include "Core/Network/IpAddress.h"

#if RIO_PLATFORM_POSIX
	#include <errno.h>
	#include <fcntl.h>      // fcntl
	#include <netinet/in.h> // htons, htonl, ...
	#include <sys/socket.h>
	#include <unistd.h>     // close

	#define SOCKET_ERROR (-1)
	#define closesocket close
	#define WSAEADDRINUSE EADDRINUSE
	#define WSAECONNREFUSED ECONNREFUSED
	#define WSAETIMEDOUT ETIMEDOUT
	#define WSAEWOULDBLOCK EWOULDBLOCK
#endif // RIO_PLATFORM_POSIX

namespace Rio
{

namespace
{

	inline int getLastError()
	{
#if RIO_PLATFORM_POSIX
		return errno;
#elif RIO_PLATFORM_WINDOWS
		return WSAGetLastError();
#endif
	}

}

namespace SocketInternalFn
{
	SOCKET open()
	{
		SOCKET socketIndex = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		RIO_ASSERT(socketIndex >= 0, "socket: getLastError() = %d", getLastError());
		return socketIndex;
	}

	AcceptResult accept(SOCKET socket, TcpSocket& tcpSocket)
	{
		SOCKET err = ::accept(socket, NULL, NULL);

		AcceptResult acceptResult;
		acceptResult.error = AcceptResult::SUCCESS;

		if (err == INVALID_SOCKET)
		{
			if (getLastError() == WSAEWOULDBLOCK)
			{
				acceptResult.error = AcceptResult::NO_CONNECTION;
			}
			else
			{
				acceptResult.error = AcceptResult::UNKNOWN;
			}
		}
		else
		{
			tcpSocket.socket = err;
		}

		return acceptResult;
	}

	ReadResult read(SOCKET socket, void* data, uint32_t size)
	{
		ReadResult readResult;
		readResult.error = ReadResult::SUCCESS;
		readResult.bytesRead = 0;

		uint32_t toRead = size;

		while (toRead > 0)
		{
			int bytesRead = ::recv(socket
				, (char*)data + readResult.bytesRead
				, toRead
				, 0
				);

			if (bytesRead == SOCKET_ERROR)
			{
				if (getLastError() == WSAEWOULDBLOCK)
				{
					readResult.error = ReadResult::WOULDBLOCK;
				}
				else if (getLastError() == WSAETIMEDOUT)
				{
					readResult.error = ReadResult::TIMEOUT;
				}
				else
				{
					readResult.error = ReadResult::UNKNOWN;
				}
				return readResult;
			}
			else if (bytesRead == 0)
			{
				readResult.error = ReadResult::REMOTE_CLOSED;
				return readResult;
			}

			toRead -= bytesRead;
			readResult.bytesRead += bytesRead;
		}

		return readResult;
	}

	WriteResult write(SOCKET socket, const void* data, uint32_t size)
	{
		WriteResult writeResult;
		writeResult.error = WriteResult::SUCCESS;
		writeResult.bytesWritten = 0;

		uint32_t toWrite = size;

		while (toWrite > 0)
		{
			int bytesWritten = ::send(socket
				, (char*)data + writeResult.bytesWritten
				, toWrite
				, 0
				);

			if (bytesWritten == SOCKET_ERROR)
			{
				if (getLastError() == WSAEWOULDBLOCK)
				{
					writeResult.error = WriteResult::WOULDBLOCK;
				}
				else if (getLastError() == WSAETIMEDOUT)
				{
					writeResult.error = WriteResult::TIMEOUT;
				}
				else
				{
					writeResult.error = WriteResult::UNKNOWN;
				}
				return writeResult;
			}
			else if (bytesWritten == 0)
			{
				writeResult.error = WriteResult::REMOTE_CLOSED;
				return writeResult;
			}

			toWrite -= bytesWritten;
			writeResult.bytesWritten += bytesWritten;
		}
		return writeResult;
	}

} // namespace SocketInternalFn

TcpSocket::TcpSocket()
{
}

void TcpSocket::close()
{
	if (socket != INVALID_SOCKET)
	{
		::closesocket(socket);
		socket = INVALID_SOCKET;
	}
}

ConnectResult TcpSocket::connect(const IpAddress& ip, uint16_t port)
{
	close();
	socket = SocketInternalFn::open();

	sockaddr_in addrIn;
	addrIn.sin_family = AF_INET;
	addrIn.sin_addr.s_addr = htonl(ip.getAddressAsUint32());
	addrIn.sin_port = htons(port);

	int err = ::connect(socket, (const sockaddr*)&addrIn, sizeof(sockaddr_in));

	ConnectResult connectResult;
	connectResult.error = ConnectResult::SUCCESS;

	if (err == SOCKET_ERROR)
	{
		if (getLastError() == WSAECONNREFUSED)
		{
			connectResult.error = ConnectResult::REFUSED;
		}
		else if (getLastError() == WSAETIMEDOUT)
		{
			connectResult.error = ConnectResult::TIMEOUT;
		}
		else
		{
			connectResult.error = ConnectResult::UNKNOWN;
		}
	}

	return connectResult;
}

BindResult TcpSocket::bind(uint16_t port)
{
	close();
	socket = SocketInternalFn::open();
	setReuseAddress(true);

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	int err = ::bind(socket, (const sockaddr*)&address, sizeof(sockaddr_in));

	BindResult bindResult;
	bindResult.error = BindResult::SUCCESS;

	if (err == SOCKET_ERROR)
	{
		if (getLastError() == WSAEADDRINUSE)
		{
			bindResult.error = BindResult::ADDRESS_IN_USE;
		}
		else
		{
			bindResult.error = BindResult::UNKNOWN;
		}
	}

	return bindResult;
}

void TcpSocket::listen(uint32_t max)
{
	int err = ::listen(socket, max);
	RIO_ASSERT(err == 0, "listen: getLastError() = %d", getLastError());
	RIO_UNUSED(err);
}

AcceptResult TcpSocket::accept(TcpSocket& c)
{
	setIsBlocking(true);
	return SocketInternalFn::accept(socket, c);
}

AcceptResult TcpSocket::acceptNonBlocking(TcpSocket& c)
{
	setIsBlocking(false);
	return SocketInternalFn::accept(socket, c);
}

ReadResult TcpSocket::read(void* data, uint32_t size)
{
	setIsBlocking(true);
	return SocketInternalFn::read(socket, data, size);
}

ReadResult TcpSocket::readNonBlocking(void* data, uint32_t size)
{
	setIsBlocking(false);
	return SocketInternalFn::read(socket, data, size);
}

WriteResult TcpSocket::write(const void* data, uint32_t size)
{
	setIsBlocking(true);
	return SocketInternalFn::write(socket, data, size);
}

WriteResult TcpSocket::writeNonBlocking(const void* data, uint32_t size)
{
	setIsBlocking(false);
	return SocketInternalFn::write(socket, data, size);
}

void TcpSocket::setIsBlocking(bool isBlocking)
{
#if RIO_PLATFORM_POSIX
	int flags = fcntl(socket, F_GETFL, 0);
	fcntl(this->socket, F_SETFL, isBlocking ? (flags & ~O_NONBLOCK) : O_NONBLOCK);
#elif RIO_PLATFORM_WINDOWS
	u_long nonBlocking = isBlocking ? 0 : 1;
	int err = ioctlsocket(this->socket, FIONBIO, &nonBlocking);
	RIO_ASSERT(err == 0, "ioctlsocket: getLastError() = %d", getLastError());
	RIO_UNUSED(err);
#endif
}

void TcpSocket::setReuseAddress(bool reuse)
{
	int optval = (int)reuse;
	int err = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	RIO_ASSERT(err == 0, "setsockopt: getLastError() = %d", getLastError());
	RIO_UNUSED(err);
}

void TcpSocket::setTimeout(uint32_t seconds)
{
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	int err = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	RIO_ASSERT(err == 0, "setsockopt: getLastError(): %d", getLastError());
	err = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	RIO_ASSERT(err == 0, "setsockopt: getLastError(): %d", getLastError());
	RIO_UNUSED(err);
}

} // namespace Rio
