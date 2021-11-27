#pragma once
#include "SFML/Network.hpp"
#include "Features/Feature.hpp"

class TasSocket : public Feature {

private:
	bool enabled;
	sf::TcpSocket client;
	bool clientConnected;
	sf::TcpListener tcpListener;
	
	int lastReceivedTick;

public:
	TasSocket();
	void enable();
	void accept();
	void receive();
	void disable();
};

extern TasSocket *tasSocket;