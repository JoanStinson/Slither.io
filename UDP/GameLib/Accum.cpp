#include "Accum.h"

Accum::Accum() {}

Accum::~Accum() {}

enum PacketType { HELLO, WELCOME, NEWPLAYER, CONTADOR, MOVE, PING, GETCOIN, RECEIVECOIN, FINDEPARTIDA, EMPTY, ACK };

Accum::Accum(int id, int idmove, int deltax, int deltay, int posx, int posy) {
	this->id = id;
	this->idmove = idmove;
	this->deltax = deltax;
	this->deltay = deltay;
	this->posx = posx;
	this->posy = posy;
}

sf::Packet Accum::AccumPacket() {
	sf::Packet packet;
	//canviar por move
	enum PacketType enumContador = PacketType::MOVE;

	posx += deltax;
	posy += deltay;

	packet << enumContador << deltax << deltay << posx << posy << id << idmove;
	return packet;
}