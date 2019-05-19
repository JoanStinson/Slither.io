#pragma once
#include <SFML/Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>

enum PacketType { HELLO, WELCOME, NEWPLAYER, CONTADOR, MOVE, PING, GETBALL, RECEIVEBALL, FINDEPARTIDA, EMPTY, ACK };

struct Player {
	sf::IpAddress ip;
	unsigned short port;

	sf::Vector2i pos;
	sf::Vector2i size = sf::Vector2i(50, 100);
	sf::Color color;
	sf::Clock clock;
	int ID = 0;
	int score = 0;
	bool connected;
};

class Accum {

public:

	Accum();
	~Accum();
	Accum(int id, int idmove, int deltax, int deltay, int posx, int posy);
	sf::Packet AccumPacket();

	int id;
	int idmove;
	int idnum;
	int deltax;
	int deltay;
	int posx;
	int posy;
};

