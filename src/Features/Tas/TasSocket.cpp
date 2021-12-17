#include "TasSocket.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Event.hpp"

#include <sstream>
#include <vector>

#define RECEIVE_INTERVAL 60

TasSocket *tasSocket;

TasSocket::TasSocket()
	: enabled(false) 
	, clientConnected(false)
	, lastReceivedTick(0) {
	this->hasLoaded = true;
}

void TasSocket::enable() {
	if (this->tcpListener.getLocalPort() != 0) return;

	enabled = true;

	console->Print("Starting tcp server...\n");

	this->client.setBlocking(false);

	if (tcpListener.listen(64253, sf::IpAddress("127.0.0.1")) != sf::Socket::Done) {
		console->Print("TCP Listener listen failed!\n");
		return;
	}
	this->tcpListener.setBlocking(false);

	console->Print("TCP Listener listening! Port: %hu\n", this->tcpListener.getLocalPort());
}

void TasSocket::accept() {
	if (this->clientConnected) return;

	if (tcpListener.accept(this->client) != sf::Socket::Done) return;
	console->Print("Client connected!\n");
	this->clientConnected = true;
}

void TasSocket::receive() {
	if (!this->enabled || !this->clientConnected) return;

	lastReceivedTick = engine->GetTick();
	size_t received;
	char data[100];
	auto status = client.receive(data, 100, received);
	if (status == sf::Socket::NotReady) return;
	else if (status == sf::Socket::Disconnected) {
		console->Print("Client disconnected!");
		this->clientConnected = false;
		return;
	}
	console->Print("Packet received: %s Received: %lu\n", data, received);

	std::vector<std::string> arguments;
	std::istringstream stream(data);
	std::string string;
	while (std::getline(stream, string, ' '))
		arguments.push_back(string);

	if (arguments[0] == "play" && arguments.size() == 2) {
		const char *command = std::string("sar_tas_play ").append(arguments[1]).c_str();

		//console->Print("Executing command: %s\n", command);
		engine->ExecuteCommand(command);
	}
	else if (arguments[0] == "stop")
		engine->ExecuteCommand("sar_tas_stop");
	else
		console->Print("Unsupported arguments: %s (Count: %lu)\n", arguments[0].c_str(), arguments.size());
}

void TasSocket::disable() {
	this->enabled = false;

	client.disconnect();
	tcpListener.close();
	this->clientConnected = false;

	console->Print("Closed TCP socket!\n");
}

// FIXME: Use some different event that also works in the menu
ON_EVENT(POST_TICK) {
	tasSocket->accept();
	tasSocket->receive();
}

CON_COMMAND(sar_tas_socket_enable, "") {
	tasSocket->enable();
}

CON_COMMAND(sar_tas_socket_disable, "") {
	tasSocket->disable();
}