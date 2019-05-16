#pragma once
#include <SFML\Network.hpp>
#include <iostream>


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

