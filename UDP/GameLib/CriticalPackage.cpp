#pragma once
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>

enum PacketType { HELLO, WELCOME, NEWPLAYER, CONTADOR, MOVE, PING, GETBALL, RECEIVEBALL, FINDEPARTIDA, EMPTY, ACK };

struct Player {
	sf::IpAddress ip;
	unsigned short port;

	std::map<int, sf::Packet> aCriticals;
	sf::Vector2i pos;
	sf::Vector2i size = sf::Vector2i(50, 100);
	sf::Color color;
	sf::Clock clock;
	int ID = 0;
	int score = 0;
	bool connected;
	bool ack;
};