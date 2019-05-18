#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <Accum.h>
#include <list>
#include <random>
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>       

#define IP_SERVER "localhost"
#define PORT_SERVER 50000
#define PERCENT_PACKETLOSS 5

struct Player {
	sf::IpAddress ip;
	unsigned short port;

	int ID;
	bool connected;
	sf::Vector2i pos;
	int score = 0;
	sf::Color color;
};

sf::UdpSocket sock;
std::vector<Player> aPlayers;
std::list<Accum> aAccum;
sf::Vector2f coin_pos;
std::string strEP;
const int INITPOS[4] = { 200, 300, 400, 500 };
int myId = -1, idmove = 0, deltax = 0, deltay = 0, idWinner = 0, speed = 4;
bool hello = false, initPlay = true, disconnected = false, empezarPartida = false, disappearText,
winner = false, coinPositioning = true, activarPerdida;

// Sirve para ahorrar código a la hora de dibujar jugadores por pantalla
void DrawPlayer(sf::RenderWindow& window, sf::Color color, sf::Vector2i pos) {
	sf::RectangleShape rectAvatar(sf::Vector2f(60, 60));
	rectAvatar.setFillColor(color);
	rectAvatar.setPosition(sf::Vector2f(pos.x, pos.y)); 
	window.draw(rectAvatar);
}

// Dibuja texto dando instrucciones
void DrawTextEP(sf::RenderWindow& window, sf::Clock clock) {
	
	if (empezarPartida && clock.getElapsedTime().asSeconds() < 5) {
		sf::Font font;
		std::string pathFont = "arial.ttf";
		if (!font.loadFromFile(pathFont))
			std::cout << "No se pudo cargar la fuente" << std::endl;

		sf::Text text(strEP.c_str() + aPlayers[0].ID, font);
		text.setPosition(sf::Vector2f(60.f, 40.f));
		text.setCharacterSize(22);
		text.setFillColor(sf::Color::White);
		text.setStyle(sf::Text::Bold);
		window.draw(text);
	}
	else disappearText = true;
}

static float GetRandomFloat() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.f, 1.f);
	return dis(gen);
}

void DibujaSFML() {
	sf::Clock clCount;
	sf::Clock clMove;
	sf::Clock clDisconnected;
	sf::Clock stclock;
	sf::Clock clockMove;
	sf::RenderWindow window(sf::VideoMode(800, 600), "Cargando...");

	while (window.isOpen()) {
		sf::Event event;

		// Este primer WHILE es para controlar los eventos del mouse
		while (window.pollEvent(event)) {

			switch (event.type) {

				case sf::Event::Closed:
					window.close();
					break;

				case sf::Event::KeyPressed:

					if (initPlay && aPlayers[0].connected) {

						if (event.key.code == sf::Keyboard::Left) {
							if (aPlayers[0].pos.x - speed >= 0) {
								deltax -= speed;
								aPlayers[0].pos.x -= speed;
							}
						}
						else if (event.key.code == sf::Keyboard::Right) {
							if (aPlayers[0].pos.x + speed <= 740) {
								deltax += speed;
								aPlayers[0].pos.x += speed;
							}
						}
						else if (event.key.code == sf::Keyboard::Up) {
							if (aPlayers[0].pos.y - speed >= 0) {
								deltay -= speed;
								aPlayers[0].pos.y -= speed;
							}
						}
						else if (event.key.code == sf::Keyboard::Down) {
							if (aPlayers[0].pos.y + speed <= 540) {
								deltay += speed;
								aPlayers[0].pos.y += speed;
							}
						}
					}
					break;

				default:
					break;
			}
		}

		// Mientras no recibimos señal por parte del servidor enviamos hello cada 550 ms
		if (!hello) {
			if (clCount.getElapsedTime().asMilliseconds() >= 550) {
				sf::Packet pckHello;
				enum PacketType enumSend = PacketType::HELLO;
				pckHello << enumSend;
				sock.send(pckHello, IP_SERVER, PORT_SERVER);
				clCount.restart();
			}
		}
		
		sf::Packet pck;
		sf::IpAddress ipRem;
		unsigned short portRem;
		sf::Socket::Status status = sock.receive(pck, ipRem, portRem);

		if (status == sf::Socket::Done) {
			
			int intReceive;
			enum PacketType enumReceive;
			pck >> intReceive;
			enumReceive = (PacketType)intReceive;

			switch (enumReceive) {

				case WELCOME: {

					int myPosX = 0;
					int myPosY = 0;
					int size = 0;
					pck >> myId >> myPosX >> myPosY >> coin_pos.x >> coin_pos.y >> size;

					std::cout << "ID: " << myId <<" "<< myPosX<<" " << " "<<myPosY;

					if (!hello) {
						
						// Creamos nuestro jugador
						Player myPlayer;
						myPlayer.ID = myId;
						myPlayer.pos.x = myPosX;
						myPlayer.pos.y = myPosY;
						myPlayer.connected = true;
						myPlayer.color = sf::Color::Blue;
						aPlayers.push_back(myPlayer);

						// Creamos los jugadores que ya estaban conectados
						if (size > 0) {
							std::cout << "pepe" << std::endl;
							int id = 0, posx = 0, posy = 0;
							for (int i = 0; i < size-1; i++) {
								Player newPlayer;
								pck >> id >> posx >> posy;
								std::cout << " "<<id << " "<<posx << " "<<posy;
								newPlayer.ID = id;
								newPlayer.pos.x = posx;
								newPlayer.pos.y = posy;
								newPlayer.connected = true;
								newPlayer.color = sf::Color::Red;
								aPlayers.push_back(newPlayer);
							}
						}

						std::cout << "Mensaje del servidor: WELCOME!" << std::endl;
						std::cout << "Soy el jugador: " << myId << " con pos: " << myPosX << ", " << myPosY << std::endl;
						window.setTitle("Slither.io - Jugador " + std::to_string(myId));
						hello = true;
					}

					if (activarPerdida) {
						float rndPacketLoss = GetRandomFloat();
						if (rndPacketLoss < PERCENT_PACKETLOSS) {
							//InputMemoryBitStream imbs(_message, bytesReceived);
							enum PacketType pt = PacketType::EMPTY;
							//imbs.Read(&pt, 3);
							std::cout << rndPacketLoss << "Simulamos que se pierde msg de tipo " << pt << " - " << std::endl;
							//return -1;
						}
					}

				}
				break;

				case NEWPLAYER: {
					
					
					int id;
					int posx;
					int posy;
					pck >> id >> posx >> posy;

					std::cout << "ID NEW: " << id << std::endl;

					//if (id != 0) {
						//std::cout << "ID NEWPLAYER: " << id;
						Player newPlayer;
						newPlayer.ID = id;
						newPlayer.pos.x = posx;
						newPlayer.pos.y = posy;
						newPlayer.connected = true;
						newPlayer.color = sf::Color::Red;

						bool push = true;
						for (int i = 0; i < aPlayers.size(); i++) {
							if (newPlayer.ID == aPlayers[i].ID)
								push = false;
						}

						if (push)
						aPlayers.push_back(newPlayer);

						std::cout << "SIZE: " << aPlayers.size() << std::endl;
					//}


					//if (aPlayers.size() == 4) 
						//initPlay = true;
											
				}
				break;

				case CONTADOR: {

					strEP += "  ";
					pck >> strEP;

					empezarPartida = true;
					stclock.restart();
				}
				break;

				case MOVE: {
					std::cout << "SIZE: " << aPlayers.size() << std::endl;
					// Actualizamos solo el jugador que se ha movido
					int size = 0;
					int id = 0;
					pck >> size >> id;

					for (int i = 0; i < size; i++) {
						if (aPlayers[i].ID == id)
							pck >> aPlayers[i].pos.x >> aPlayers[i].pos.y;
					}
					

					std::cout << "id " << id << std::endl;
				}
				break;

				//TODO paquetes criticos!!
				case ACK: {
					
				}
				break;

				case PING: {

					int id;
					int size;
					pck >> size >> id;
					std::cout << "Se ha desconectado el jugador " << id << "!" << std::endl;

					if (aPlayers[0].ID == id) {
						aPlayers[0].connected = false;
						clDisconnected.restart();
						disconnected = true;
					}
				}
				break;

				case RECEIVECOIN: {

					pck >> coin_pos.x >> coin_pos.y;
					coinPositioning = true;
				}
				break;

				case FINDEPARTIDA: {

					int id;
					pck >> id;
					std::cout << "El jugador " << id << " ha ganado la partida!" << std::endl;
					idWinner = id;
					winner = true;
				}
				break;

				default:
				break;
			}

		}

		window.clear();

		if (disconnected) {

			// Mostrar mensaje por pantalla
			if (!aPlayers[0].connected) {
				std::cout << "Disconnected" << std::endl;
				sf::Font font;
				std::string pathFont = "arial.ttf";
				if (!font.loadFromFile(pathFont))
					std::cout << "No se pudo cargar la fuente" << std::endl;

				sf::Text text("  Disconnected player" + aPlayers[0].ID, font);
				text.setPosition(sf::Vector2f(260, 50));
				text.setCharacterSize(24);
				text.setFillColor(sf::Color::Cyan);
				text.setStyle(sf::Text::Bold);
				window.draw(text);

				if (clDisconnected.getElapsedTime().asSeconds() >= 5){
					disconnected = false;
					clDisconnected.restart();
				}
			}
			
		}

		// Fondo
		sf::Texture texture;
		texture.loadFromFile("kirby.png");

		sf::Sprite sprite(texture);
		sprite.setScale(1.f, 1.f);
		sprite.setPosition(0.f, 0.f);
		window.draw(sprite);

		// Pintar jugadores
		if (aPlayers.size() > 0) {
			for (int i = 0; i < aPlayers.size(); i++) {
				if (aPlayers[i].connected)
					DrawPlayer(window, aPlayers[i].color, aPlayers[i].pos);
			}
		}
	
		if (empezarPartida)
			DrawTextEP(window, stclock);

		// Si el jugador es mou li enviem cada 200ms la llista amb tota la acumulació (Acumulem cada cop que fem apretem una tecla)
		if (empezarPartida && clockMove.getElapsedTime().asMilliseconds() >= 200 && (deltax != 0 || deltay != 0)) {

			if (aPlayers[0].pos.x - coin_pos.x < 1.f && aPlayers[0].pos.y - coin_pos.y < 1.f && coinPositioning) {
				sf::Packet pa;
				enum PacketType enumCoin = PacketType::GETCOIN;
				pa << enumCoin << myId;
				sock.send(pa, IP_SERVER, PORT_SERVER);
				aPlayers[0].score++;
				std::cout << "Has puntuado! Tienes un total de " << aPlayers[0].score << " puntos!" << std::endl;
				coinPositioning = false;
			}

			int prevX = aPlayers[0].pos.x;
			int prevY = aPlayers[0].pos.y;
			idmove++;

			std::cout << "El jugador " << myId << " se ha movido! Delta Pos: (" << deltax << ", " << deltay << ") PosReal: (" << aPlayers[0].pos.x << ", " << aPlayers[0].pos.y << ")" << std::endl;

			Accum accum(myId, idmove, deltax, deltay, aPlayers[0].pos.x, aPlayers[0].pos.y);
			aAccum.push_back(accum);

			sf::Packet pack = accum.AccumPacket();
			sock.send(pack, IP_SERVER, PORT_SERVER);

			deltax = 0;
			deltay = 0;
			clockMove.restart();
		}

		// Moneda
		if (disappearText && !winner) {
			sf::CircleShape coin(25);
			coin.setFillColor(sf::Color::White);
			coin.setPosition(coin_pos);
			window.draw(coin);
		}

		// Ganador
		if (winner) {
			sf::Font font;
			std::string pathFont = "arial.ttf";
			if (!font.loadFromFile(pathFont))
				std::cout << "No se pudo cargar la fuente" << std::endl;
			std::string ss = "                         El jugador " + std::to_string(idWinner) + " ha ganado la partida!";
			sf::Text text(ss.c_str() + aPlayers[0].ID, font);
			text.setPosition(sf::Vector2f(60.f, 40.f));
			text.setCharacterSize(22);
			text.setFillColor(sf::Color::White);
			text.setStyle(sf::Text::Bold);
			window.draw(text);
		}

		window.display();
	}
}

int main() {

	sock.setBlocking(false);
	DibujaSFML();

	return 0;
}