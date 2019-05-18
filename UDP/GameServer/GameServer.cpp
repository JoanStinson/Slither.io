#include <SFML\Network.hpp>
#include <Accum.h>
#include <iostream>
#include <ctime>
#include <list>
#include <random>
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>       

struct Player {
	sf::IpAddress ip;
	unsigned short port;

	int ID = 0; 
	sf::Vector2i pos;
	bool connected;
	sf::Clock clock;
	bool newPlayer;
	int score = 0;
};

std::vector<Player> aPlayers;
std::vector<Accum> aAccum;
std::vector<Accum> agrupadosAccum;
sf::Vector2f coin_pos;
sf::Vector2i pos;
int idJugador = 1, initPos = 200, id, size, deltax, deltay, idnum, timeToDisconnectPlayer = 180; // Default = 180
bool firstTime;

bool FindPlayer(unsigned short portRem) {
	bool result = true;

	for (unsigned int i = 0; i < aPlayers.size(); i++) {
		if (aPlayers[i].port != portRem) 
			result = false;
		if (aPlayers[i].port == portRem) {
			result = true;
			break;
		}
	}
	return result;
}

void SendNonBlocking(sf::UdpSocket* socket, sf::Packet packet, sf::IpAddress ip, unsigned short port) {
	sf::Socket::Status status = socket->send(packet, ip, port);

	while (status == sf::Socket::Partial) {
		status = socket->send(packet, ip, port);
	}
}

sf::Packet& operator<<(sf::Packet& packet, std::vector<int>& vec) {
	for (int i = 0; i < vec.size(); i++) 
		packet << vec[i];
	
	return packet;
}

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

int main() {

	sf::UdpSocket sock;
	sock.setBlocking(false);
	sf::Socket::Status status = sock.bind(50000);

	if (status != sf::Socket::Done) {
		std::cout << "Error al vincular\n";
		system("pause");
		exit(0);
	}
	sf::Clock clock;
		sf::Clock clockMove;

	do {

		if (aPlayers.size() >= 4) {

			if (!firstTime) {

				// Por defecto acumuladores
				Accum accum1(1, 1, 0, 0, 0, 0);
				Accum accum2(2, 1, 0, 0, 0, 0);
				Accum accum3(3, 1, 0, 0, 0, 0);
				Accum accum4(4, 1, 0, 0, 0, 0);

				agrupadosAccum.push_back(accum1);
				agrupadosAccum.push_back(accum2);
				agrupadosAccum.push_back(accum3);
				agrupadosAccum.push_back(accum4);

				firstTime = true;
			}

			if (aPlayers[0].connected && aPlayers[0].clock.getElapsedTime().asSeconds() > timeToDisconnectPlayer) {
				enum PacketType enumDisconnect = PacketType::PING;
				sf::Packet pckDisconnectPlayer;
				int size = aPlayers.size();
				// usuario a notificar del que se desconecta de la partida
				aPlayers[0].connected = false;
				pckDisconnectPlayer << enumDisconnect << size << aPlayers[0].ID;

				for (unsigned int i = 0; i < size; i++)
					SendNonBlocking(&sock, pckDisconnectPlayer, aPlayers[i].ip, aPlayers[i].port);
			}

			if (aPlayers[1].connected && aPlayers[1].clock.getElapsedTime().asSeconds() > timeToDisconnectPlayer) {
				enum PacketType enumDisconnect = PacketType::PING;
				sf::Packet pckDisconnectPlayer;
				int size = aPlayers.size();
				// usuario a notificar del que se desconecta de la partida
				aPlayers[1].connected = false;
				pckDisconnectPlayer << enumDisconnect << size << aPlayers[1].ID;

				for (unsigned int i = 0; i < size; i++)
					SendNonBlocking(&sock, pckDisconnectPlayer, aPlayers[i].ip, aPlayers[i].port);
			}

			if (aPlayers[2].connected && aPlayers[2].clock.getElapsedTime().asSeconds() > timeToDisconnectPlayer) {
				enum PacketType enumDisconnect = PacketType::PING;
				sf::Packet pckDisconnectPlayer;
				int size = aPlayers.size();
				// usuario a notificar del que se desconecta de la partida
				aPlayers[2].connected = false;
				pckDisconnectPlayer << enumDisconnect << size << aPlayers[2].ID;

				for (unsigned int i = 0; i < size; i++)
					SendNonBlocking(&sock, pckDisconnectPlayer, aPlayers[i].ip, aPlayers[i].port);
			}

			if (aPlayers[3].connected && aPlayers[3].clock.getElapsedTime().asSeconds() > timeToDisconnectPlayer) {
				enum PacketType enumDisconnect = PacketType::PING;
				sf::Packet pckDisconnectPlayer;
				int size = aPlayers.size();
				// usuario a notificar del que se desconecta de la partida
				aPlayers[3].connected = false;
				pckDisconnectPlayer << enumDisconnect << size << aPlayers[3].ID;

				for (unsigned int i = 0; i < size; i++)
					//hacer funcion send para nonblocking y mirar partial
					SendNonBlocking(&sock, pckDisconnectPlayer, aPlayers[i].ip, aPlayers[i].port);
			}
		}
			
		sf::Packet pck;
		sf::IpAddress ipRem; 
		unsigned short portRem;
		status = sock.receive(pck, ipRem, portRem); 

		if (status == sf::Socket::Done) {
				
			int intReceive;
			enum PacketType enumReceive;
			pck >> intReceive;
			enumReceive = (PacketType)intReceive;
				
			enum PacketType enumWelcome = PacketType::WELCOME;
			enum PacketType enumNewPlayer = PacketType::NEWPLAYER;
			enum PacketType enumContador = PacketType::CONTADOR;

			switch (enumReceive) {

				case HELLO: {

					if (aPlayers.empty()) {

						coin_pos.x = rand() % 400 + 300;
						coin_pos.y = rand() % 400 + 300;

						std::cout << "Mensaje del cliente: " << "HELLO!" << " ip: " << ipRem << " puerto: " << portRem << std::endl;

						Player firstPlayer;
						firstPlayer.ip = ipRem;
						firstPlayer.port = portRem;

						firstPlayer.ID = idJugador;
						firstPlayer.pos.x = initPos;
						firstPlayer.pos.y = 500;

						firstPlayer.connected = true;
						aPlayers.push_back(firstPlayer);

						sf::Packet pckSend;
						pckSend << enumWelcome << firstPlayer.ID << firstPlayer.pos.x << firstPlayer.pos.y << coin_pos.x << coin_pos.y;
						SendNonBlocking(&sock, pckSend, ipRem, portRem);
							
						aPlayers[idJugador - 1].clock.restart();
					}

					else {

						if (!FindPlayer(portRem)) {

							std::cout << "Mensaje del cliente: " << "HELLO!" << " ip: " << ipRem << " puerto: " << portRem << std::endl;

							Player newPlayer;
							newPlayer.ip = ipRem;
							newPlayer.port = portRem;

							idJugador++;
							initPos += 100;

							newPlayer.ID = idJugador;
							newPlayer.pos.x = initPos;
							newPlayer.pos.y = 500;

							newPlayer.connected = true;
							aPlayers.push_back(newPlayer);

							// Le damos la id y pos al jugador correspondiente
							sf::Packet pckWelcome;
							pckWelcome << enumWelcome << newPlayer.ID << newPlayer.pos.x << newPlayer.pos.y << coin_pos.x << coin_pos.y;
							SendNonBlocking(&sock, pckWelcome, newPlayer.ip, newPlayer.port);

							// Notificamos a los otros jugadores del nuevo jugador
							sf::Packet pckNewPlayer;
							int size = aPlayers.size();
							pckNewPlayer << enumNewPlayer << size;
							for (unsigned int i = 0; i < size; i++)
								pckNewPlayer << aPlayers[i].ID << aPlayers[i].pos.x << aPlayers[i].pos.y;

							for (unsigned int i = 0; i < size; i++)
								SendNonBlocking(&sock, pckNewPlayer, aPlayers[i].ip, aPlayers[i].port);

							aPlayers[idJugador - 1].clock.restart();

							if (size == 4) {
								aPlayers[0].clock.restart();
								aPlayers[1].clock.restart();
								aPlayers[2].clock.restart();
								aPlayers[3].clock.restart();

								sf::Packet p;
								std::string s = "    La partida empieza! Gana quien consiga 7 monedas antes! \n Si un jugador no se mueve en Xs se desconecta!";

								p << enumContador << s;
								for (unsigned int i = 0; i < size; i++)
									SendNonBlocking(&sock, p, aPlayers[i].ip, aPlayers[i].port);
										
							}

						}
						// Si el cliente ya existe
						else {
							Player repeatPlayer;
							// Buscamos ese puerto en el array y recogemos la id correspondiente
							for (unsigned int i = 0; i < aPlayers.size(); i++) {
								if (aPlayers[i].port == portRem) {
									repeatPlayer.ID = aPlayers[i].ID;
									repeatPlayer.pos = aPlayers[i].pos;
									repeatPlayer.ip = aPlayers[i].ip;
									repeatPlayer.port = aPlayers[i].port;
								}
							}

							// Le volvemos a enviar el mismo mensaje
							// Le damos la id y pos al jugador correspondiente solo a ese cliente, porque no es un newplayer
							sf::Packet pckWelcome;
							pckWelcome << enumWelcome << repeatPlayer.ID << repeatPlayer.pos.x << repeatPlayer.pos.y << coin_pos.x << coin_pos.y;
							SendNonBlocking(&sock, pckWelcome, repeatPlayer.ip, repeatPlayer.port);
						}
					}
				}
				break;

				case ACK: {

				}
				break; 

				case MOVE: {

					size = aPlayers.size();
					pck >> deltax >> deltay >> pos.x >> pos.y >> id >> idnum;

					agrupadosAccum[id-1].idmove = idnum;
					agrupadosAccum[id-1].deltax += deltax;
					agrupadosAccum[id-1].deltay += deltay;
					agrupadosAccum[id-1].posx = pos.x;
					agrupadosAccum[id-1].posy = pos.y;

					aPlayers[id - 1].clock.restart();
				}
				break;

				case GETCOIN: {

					pck >> id;
					aPlayers[id - 1].score += 1;
					std::cout << std::endl << "El jugador " << id << " tiene " << aPlayers[id - 1].score << " puntos!" << std::endl << std::endl;

					if (aPlayers[id - 1].score >= 7) {
						enum PacketType pa = PacketType::FINDEPARTIDA;
						sf::Packet pp;
						pp << pa << id;

						for (unsigned int i = 0; i < size; i++)
							sock.send(pp, aPlayers[i].ip, aPlayers[i].port);
					}
					else {
						enum PacketType pa = PacketType::RECEIVECOIN;
						sf::Packet pp;
						coin_pos.x = RandomFloat(100, 400);
						coin_pos.y = RandomFloat(100, 400);
						pp << pa << coin_pos.x << coin_pos.y;

						for (unsigned int i = 0; i < size; i++)
							SendNonBlocking(&sock, pp, aPlayers[i].ip, aPlayers[i].port);
					}
				}
				break;

				default:
					break;
			}

			if (clockMove.getElapsedTime().asMilliseconds() >= 100 && agrupadosAccum.size() > 0) {
				// Asignar pos al player que le corresponde
				for (unsigned int i = 0; i < size; i++) {
					if (id == aPlayers[i].ID) {
						aPlayers[i].pos.x = agrupadosAccum[id-1].posx;
						aPlayers[i].pos.y = agrupadosAccum[id-1].posy;
					}
				}

				// Enviar pos a los otros players
				for (int i = 0; i < size; i++) {

					if (aPlayers[i].pos.x == -1 || aPlayers[i].pos.y == -1)
						break;

					std::cout << "Se intenta la pos " << aPlayers[i].pos.x << std::endl;

					if (aPlayers[i].pos.y <= 540 && aPlayers[i].pos.y >= 400) {

						std::cout << "La pos " << aPlayers[i].pos.x << ", " << aPlayers[i].pos.y << " es valida" << std::endl;

						sf::Packet pckSend;
						enum PacketType enumSend = PacketType::MOVE;

						pckSend << enumSend << size;

						// Primero enviamos la posición del jugador actual 
						pckSend << aPlayers[i].pos.x << aPlayers[i].pos.y; 

						// Luego la posición de los demás jugadores
						for (int j = 0; j < size; j++) {
							if (aPlayers[j].connected && aPlayers[j].ID != aPlayers[i].ID)
								pckSend << aPlayers[j].pos.x << aPlayers[j].pos.y;
						}

						if (aPlayers[i].connected) {
							SendNonBlocking(&sock, pckSend, aPlayers[i].ip, aPlayers[i].port);
						}

					}
					else std::cout << "La pos " << aPlayers[i].pos.x << ", " << aPlayers[i].pos.y << " NO es valida" << std::endl;

				}
				clockMove.restart();
			}
		}
		clock.restart();
		
	} while (true);
	sock.unbind();
	return 0;
}