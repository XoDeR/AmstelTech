#pragma once

#include "Core/Containers/HashMap.h"
#include "Core/Containers/Types.h"
#include "Core/Network/Socket.h"
#include "Core/Strings/Types.h"

namespace Rio
{

// Provides service to communicate with the engine application via TCP/IP
struct ConsoleServer
{
	using CommandFunction = void (*)(ConsoleServer& consoleServer, TcpSocket client, const char* json, void* userData);

	struct Command
	{
		CommandFunction function = nullptr;;
		void* userData = nullptr;;
	};

	TcpSocket server;
	Array<TcpSocket> clientList;
	HashMap<StringId32, Command> commandMap;

	explicit ConsoleServer(Allocator& a);

	// Listens on the given <port>
	// If <wait> is true blocks until a client is connected
	void listen(uint16_t port, bool wait);

	// Shutdowns the server
	void shutdown();

	// Collects requests from clients and processes them all
	void update();

	// Sends the given JSON-encoded string to all clients
	void send(const char* json);

	// Sends the given JSON-encoded string to <client>
	void send(TcpSocket client, const char* json);

	// Sends an error message to <client>
	void sendErrorMessage(TcpSocket client, const char* message);

	// Registers the command <type>
	void registerCommand(const char* type, CommandFunction commandFunction, void* userData);
};

namespace ConsoleServerGlobalFn
{
	void init();

	void shutdown();

} // namespace ConsoleServerGlobalFn

ConsoleServer* getConsoleServerGlobal();

} // namespace Rio
