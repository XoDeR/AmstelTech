#include "Core/ConsoleServer.h"

#include "Core/Containers/HashMap.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/RJson.h"
#include "Core/Memory/TempAllocator.h"
#include "Core/Strings/StringId.h"
#include "Core/Strings/StringStream.h"

namespace Rio
{

ConsoleServer::ConsoleServer(Allocator& a)
	: clientList(a)
	, commandMap(a)
{
}

void ConsoleServer::listen(uint16_t port, bool wait)
{
	server.bind(port);

	uint32_t maxConnections = 5;
	server.listen(maxConnections);

	if (wait)
	{
		AcceptResult acceptResult;
		TcpSocket client;
		do
		{
			acceptResult = server.accept(client);
		}
		while (acceptResult.error != AcceptResult::SUCCESS);

		ArrayFn::pushBack(clientList, client);
	}
}

void ConsoleServer::shutdown()
{
	for (uint32_t i = 0; i < ArrayFn::getCount(clientList); ++i)
	{
		clientList[i].close();
	}

	server.close();
}

void ConsoleServer::send(TcpSocket client, const char* json)
{
	uint32_t length = getStrLen32(json);
	client.write(&length, 4);
	client.write(json, length);
}

void ConsoleServer::sendErrorMessage(TcpSocket client, const char* message)
{
	TempAllocator4096 tempAllocator4096;
	StringStream stringStream(tempAllocator4096);
	stringStream << "{\"type\":\"error\",\"message\":\"" << message << "\"}";
	send(client, StringStreamFn::getCStr(stringStream));
}

void ConsoleServer::send(const char* json)
{
	for (uint32_t i = 0; i < ArrayFn::getCount(clientList); ++i)
	{
		send(clientList[i], json);
	}
}

void ConsoleServer::update()
{
	TcpSocket client;
	AcceptResult acceptResult = this->server.acceptNonBlocking(client);
	if (acceptResult.error == AcceptResult::SUCCESS)
	{
		ArrayFn::pushBack(clientList, client);
	}

	TempAllocator256 alloc;
	Array<uint32_t> toRemove(alloc);

	// Update all clients
	for (uint32_t i = 0; i < ArrayFn::getCount(clientList); ++i)
	{
		for (;;)
		{
			uint32_t messageLength = 0;
			ReadResult readResult = clientList[i].readNonBlocking(&messageLength, 4);

			if (readResult.error == ReadResult::WOULDBLOCK)
			{
				break;
			}

			if (readResult.error != ReadResult::SUCCESS)
			{
				ArrayFn::pushBack(toRemove, i);
				break;
			}

			// Read message
			TempAllocator4096 ta;
			Array<char> message(ta);
			ArrayFn::resize(message, messageLength + 1);
			readResult = clientList[i].read(ArrayFn::begin(message), messageLength);
			ArrayFn::pushBack(message, '\0');

			if (readResult.error != ReadResult::SUCCESS)
			{
				ArrayFn::pushBack(toRemove, i);
				break;
			}

			// Process message
			JsonObject jsonObject(ta);
			RJsonFn::parse(ArrayFn::begin(message), jsonObject);

			Command command;
			command.function = nullptr;
			command.userData = nullptr;
			command = HashMapFn::get(commandMap, RJsonFn::parseStringId(jsonObject["type"]), command);

			if (command.function != nullptr)
			{
				command.function(*this, clientList[i], ArrayFn::begin(message), command.userData);
			}
			else
			{
				sendErrorMessage(clientList[i], "Unknown command");
			}
		}
	}

	// Remove clients
	for (uint32_t i = 0; i < ArrayFn::getCount(toRemove); ++i)
	{
		const uint32_t last = ArrayFn::getCount(clientList) - 1;
		const uint32_t clientToRemove = toRemove[i];

		clientList[clientToRemove].close();
		clientList[clientToRemove] = clientList[last];
		ArrayFn::popBack(clientList);
	}
}

void ConsoleServer::registerCommand(const char* type, CommandFunction commandFunction, void* userData)
{
	RIO_ENSURE(nullptr != type);
	RIO_ENSURE(nullptr != commandFunction);

	Command command;
	command.function = commandFunction;
	command.userData = userData;

	HashMapFn::set(commandMap, StringId32(type), command);
}

namespace ConsoleServerGlobalFn
{
	ConsoleServer* consoleServerGlobal = nullptr;

	void init()
	{
		consoleServerGlobal = RIO_NEW(getDefaultAllocator(), ConsoleServer)(getDefaultAllocator());
	}

	void shutdown()
	{
		consoleServerGlobal->shutdown();
		RIO_DELETE(getDefaultAllocator(), consoleServerGlobal);
		consoleServerGlobal = nullptr;
	}

} // namespace ConsoleServerGlobalFn

ConsoleServer* getConsoleServerGlobal()
{
	return ConsoleServerGlobalFn::consoleServerGlobal;
}

} // namespace Rio
