#include <SFML\Network.hpp>
#include <Accum.h>
#include <iostream>
#include <ctime>
#include <list>
#include <random>
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>   

#define MAX_SCORE 5

// Variables
std::vector<Player> aPlayers;
std::vector<Accum> aAccum;
sf::Vector2f ballPos;
sf::Vector2i pos;
int idJugador = 1, initPos = 40, id, size = 0, deltax, deltay, idnum, timeToDisconnectPlayer = 180; //TODO Default = 60

// Funciones
bool FindPlayer(unsigned short portRem);
void SendNonBlocking(sf::UdpSocket* socket, sf::Packet packet, sf::IpAddress ip, unsigned short port);
sf::Packet& operator<<(sf::Packet& packet, std::vector<int>& vec);
void ActualizarAccum(int id, int idnum, int deltax, int deltay, int posx, int posy);
float RandomFloat(float a, float b);

// MAIN
int main() {

	sf::Clock clockMove;
	sf::UdpSocket sock;
	sock.setBlocking(false);
	sf::Socket::Status status = sock.bind(50000);

	if (status != sf::Socket::Done) {
		std::cout << "Error al vincular\n";
		system("pause");
		exit(0);
	}

	do {
		
		// Enviamos comando de PING si un jugador no se mueven durante un tiempo
		if (size > 0) {

			for (int i = 0; i < size; i++) {

				if (aPlayers[i].connected && aPlayers[i].clock.getElapsedTime().asSeconds() > timeToDisconnectPlayer) {
					enum PacketType enumDisconnect = PacketType::PING;
					sf::Packet pckDisconnectPlayer;
					aPlayers[i].connected = false;
					pckDisconnectPlayer << enumDisconnect << size << aPlayers[i].ID;

					for (int j = 0; j < size; j++)
						SendNonBlocking(&sock, pckDisconnectPlayer, aPlayers[j].ip, aPlayers[j].port);

					aAccum.erase(aAccum.begin() + i);
				}
			}
		}
			
		sf::Packet pckReceive;
		sf::Packet pckSendMove;
		sf::IpAddress ipRem; 
		unsigned short portRem;
		status = sock.receive(pckReceive, ipRem, portRem); 

		if (status == sf::Socket::Done) {
				
			int intReceive;
			enum PacketType enumReceive;
			pckReceive >> intReceive;
			enumReceive = (PacketType)intReceive;
				
			enum PacketType enumWelcome = PacketType::WELCOME;
			enum PacketType enumNewPlayer = PacketType::NEWPLAYER;
			enum PacketType enumContador = PacketType::CONTADOR;

			switch (enumReceive) {

				case HELLO: {

					// Si es el primer jugador
					if (aPlayers.empty()) {
						std::cout << "Mensaje del cliente: " << "HELLO!" << " ip: " << ipRem << " puerto: " << portRem << std::endl;

						// Creamos el jugador
						Player firstPlayer;
						firstPlayer.ip = ipRem;
						firstPlayer.port = portRem;
						firstPlayer.ID = idJugador;
						firstPlayer.pos.x = initPos;
						firstPlayer.pos.y = 500;
						firstPlayer.connected = true;
						aPlayers.push_back(firstPlayer);
						size = aPlayers.size();

						Accum accum(idJugador, 1, 0, 0, 0, 0);
						aAccum.push_back(accum);

						// Le enviamos su informaci�n
						sf::Packet pckSend;
						ballPos.x = rand() % 400 + 300;
						ballPos.y = rand() % 400 + 300;
						pckSend << enumWelcome << firstPlayer.ID << firstPlayer.pos.x << firstPlayer.pos.y << ballPos.x << ballPos.y << 0;
						SendNonBlocking(&sock, pckSend, ipRem, portRem);
							
						// Le avisamos que ya puede empezar la partida
						sf::Packet p;
						std::string s = "    La partida empieza! Gana quien consiga "+std::to_string(MAX_SCORE)+" bolas antes! \n Si un jugador no se mueve en 60s se desconecta!";
						p << enumContador << s;
						SendNonBlocking(&sock, p, aPlayers[idJugador - 1].ip, aPlayers[idJugador - 1].port);
						aPlayers[idJugador - 1].clock.restart();
					}

					// Si no es el primer jugador
					else {

						// Comprobamos que no lo tengamos en la lista
						if (!FindPlayer(portRem)) {
							std::cout << "Mensaje del cliente: " << "HELLO!" << " ip: " << ipRem << " puerto: " << portRem << std::endl;

							// Creamos el jugador
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
							size = aPlayers.size();

							Accum accum(idJugador, 1, 0, 0, 0, 0);
							aAccum.push_back(accum);

							// Enviamos su informaci�n y los jugadores anteriores
							sf::Packet pckWelcome;
							pckWelcome << enumWelcome << newPlayer.ID << newPlayer.pos.x << newPlayer.pos.y << ballPos.x << ballPos.y << size;
							for (int i = 0; i < size - 1; i++)
								pckWelcome << aPlayers[i].ID << aPlayers[i].pos.x << aPlayers[i].pos.y;
							SendNonBlocking(&sock, pckWelcome, newPlayer.ip, newPlayer.port);

							// Le avisamos que ya puede empezar la partida
							sf::Packet p;
							std::string s = "    La partida empieza! Gana quien consiga " + std::to_string(MAX_SCORE) + " bolas antes! \n Si un jugador no se mueve en 60s se desconecta!";
							p << enumContador << s;
							SendNonBlocking(&sock, p, aPlayers[idJugador-1].ip, aPlayers[idJugador-1].port);	
							aPlayers[idJugador - 1].clock.restart();

							// Enviamos el jugador nuevo a los anteriores jugadores
							sf::Packet pckNewPlayer;
							pckNewPlayer << enumNewPlayer;
							pckNewPlayer << idJugador << initPos << 500;
							for (int i = 0; i < size - 1; i++)
								SendNonBlocking(&sock, pckNewPlayer, aPlayers[i].ip, aPlayers[i].port);
						}
						// Si lo tenemos en la lista es un jugador repetido
						else {

							// Buscamos ese puerto en el array y recogemos la id correspondiente
							Player repeatPlayer;
							for (int i = 0; i < aPlayers.size(); i++) {
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
							pckWelcome << enumWelcome << repeatPlayer.ID << repeatPlayer.pos.x << repeatPlayer.pos.y << ballPos.x << ballPos.y << 0;
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
					pckReceive >> deltax >> deltay >> pos.x >> pos.y >> id >> idnum;
					ActualizarAccum(id - 1, idnum, deltax, deltay, pos.x, pos.y);

					// Actualizamos la lista de jugadores del servidor con los agrupados del jugador
					aPlayers[id-1].pos.x = aAccum[id-1].posx;
					aPlayers[id-1].pos.y = aAccum[id-1].posy;

					// Verificamos que sea una posici�n v�lida y construimos el paquete (solo del jugador que se ha movido)
					// Para enviarlo a los dem�s jugadores cada 100 ms para reducir tr�fico
					if (aPlayers[id-1].pos.x == -1 || aPlayers[id-1].pos.y == -1) 
						break;
					std::cout << "Se intenta la pos " << aPlayers[id-1].pos.x << std::endl;

					if ((aPlayers[id-1].pos.y >= 0 && aPlayers[id-1].pos.y <= 540)
						&& aPlayers[id - 1].pos.x >= 0 && aPlayers[id - 1].pos.x <= 740) {
						std::cout << "La pos " << aPlayers[id-1].pos.x << ", " << aPlayers[id-1].pos.y << " es valida" << std::endl;

						enum PacketType enumSend = PacketType::MOVE;
						pckSendMove << enumSend << size << aPlayers[id-1].ID;
						pckSendMove << aPlayers[id-1].pos.x << aPlayers[id-1].pos.y;
					}
					else std::cout << "La pos " << aPlayers[id-1].pos.x << ", " << aPlayers[id-1].pos.y << " NO es valida" << std::endl;

					aPlayers[id - 1].clock.restart();
				}
				break;

				case GETBALL: {

					pckReceive >> id;
					aPlayers[id - 1].score += 1;
					std::cout << std::endl << "El jugador " << id << " tiene " << aPlayers[id - 1].score << " puntos!" << std::endl << std::endl;

					// Si un jugador obtiene la puntuaci�n m�xima la partida se acaba y se proclama ganador
					if (aPlayers[id - 1].score >= MAX_SCORE) {
						enum PacketType pa = PacketType::FINDEPARTIDA;
						sf::Packet pp;
						pp << pa << id;

						for (unsigned int i = 0; i < size; i++)
							sock.send(pp, aPlayers[i].ip, aPlayers[i].port);
					}
					// Se genera una nueva bola despu�s de que un jugador coja una
					else {
						enum PacketType pa = PacketType::RECEIVEBALL;
						sf::Packet pp;
						ballPos.x = RandomFloat(100, 400);
						ballPos.y = RandomFloat(100, 400);
						pp << pa << ballPos.x << ballPos.y;

						for (unsigned int i = 0; i < size; i++)
							SendNonBlocking(&sock, pp, aPlayers[i].ip, aPlayers[i].port);
					}
				}
				break;

				default:
					break;
			}

			// Enviamos el paquete de move a todos los jugadores cada 100 ms
			if (clockMove.getElapsedTime().asMilliseconds() >= 100 && aAccum.size() > 0) {

				for (int i = 0; i < aPlayers.size(); i++) {
					if (aPlayers[i].connected)
						SendNonBlocking(&sock, pckSendMove, aPlayers[i].ip, aPlayers[i].port);
				}

				pckSendMove.clear();
				clockMove.restart();
			}
		}
		
	} while (true);
	sock.unbind();
	return 0;
}

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

void ActualizarAccum(int id, int idnum, int deltax, int deltay, int posx, int posy) {
	aAccum[id].idmove = idnum;
	aAccum[id].deltax += deltax;
	aAccum[id].deltay += deltay;
	aAccum[id].posx = posx;
	aAccum[id].posy = posy;
}

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}