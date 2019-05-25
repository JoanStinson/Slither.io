#include "Accum.h"

Accum::Accum() {}

Accum::~Accum() {}

Accum::Accum(int id, int idmove, int deltax, int deltay, int posx, int posy, int speed) {
	this->id = id;
	this->idmove = idmove;
	this->deltax = deltax;
	this->deltay = deltay;
	this->posx = posx;
	this->posy = posy;
	this->speed = speed;
}

sf::Packet Accum::AccumPacket() {
	sf::Packet packet;
	enum PacketType enumContador = PacketType::MOVE;

	posx += deltax;
	posy += deltay;

	packet << enumContador << id << idmove << deltax << deltay << posx << posy << speed;
	return packet;
}