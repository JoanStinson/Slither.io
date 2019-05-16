#pragma once
#include <SFML\Network.hpp>
#include <iostream>

enum PacketType { HELLO, WELCOME, NEWPLAYER, CONTADOR, MOVE, PING, GETCOIN, RECEIVECOIN, FINDEPARTIDA, EMPTY, ACK };

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

