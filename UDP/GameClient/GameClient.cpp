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

enum PacketType { HELLO, WELCOME, NEWPLAYER, CONTADOR, MOVE, PING, GETCOIN, RECEIVECOIN, FINDEPARTIDA, EMPTY, ACK };


struct Player {
	sf::IpAddress ip;
	unsigned short port;

	int ID;
	bool connected;
	sf::Vector2i pos;
	int score = 0;
};

std::vector<Player> aPlayers;
sf::UdpSocket sock;
bool hello = false;
const int INITPOS[4] = { 200, 300, 400, 500 };
int myId = -1;
bool initPlay = false;
bool disconnected = false;
std::string strEP;
bool empezarPartida = false;
int idmove = 0;
int deltax = 0;
int deltay = 0;
bool disappearText;
std::list<Accum>aAccum;
sf::Vector2f coin_pos;
bool winner = false;
int idWinner = 0;
bool coinPositioning = true;
bool activarPerdida;

void DrawPlayer(sf::RenderWindow& window, sf::Color color, sf::Vector2i pos) {
	sf::RectangleShape rectAvatar(sf::Vector2f(60, 60));
	rectAvatar.setFillColor(color);
	rectAvatar.setPosition(sf::Vector2f(pos.x, pos.y)); 
	window.draw(rectAvatar);
}

void DrawTextEP(sf::RenderWindow& window, sf::Clock clock) {
	
	if (empezarPartida && clock.getElapsedTime().asSeconds() < 5) {
		sf::Font font;
		std::string pathFont = "arial.ttf";
		if (!font.loadFromFile(pathFont))
			std::cout << "No se pudo cargar la fuente" << std::endl;

		sf::Text text(strEP.c_str() + aPlayers[myId - 1].ID, font);
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
					if (initPlay && aPlayers[myId-1].connected) {
						if (event.key.code == sf::Keyboard::Left) {
							deltax -= 1;
							aPlayers[myId - 1].pos.x -= 1;
						}
						else if (event.key.code == sf::Keyboard::Right) {
							deltax += 1;
							aPlayers[myId - 1].pos.x += 1;

						}
						else if (event.key.code == sf::Keyboard::Up) {
							deltay -= 1;
							aPlayers[myId - 1].pos.y -= 1;
						}
						else if (event.key.code == sf::Keyboard::Down) {
							deltay += 1;
							aPlayers[myId - 1].pos.y += 1;
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

				case WELCOME:
				{
					int myPosX;
					int myPosY;
					pck >> myId >> myPosX >> myPosY >> coin_pos.x >> coin_pos.y;

					if (!hello) {
						
						aPlayers[0].ID = myId;
						aPlayers[0].pos.x = myPosX;
						aPlayers[0].pos.y = myPosY;
						aPlayers[0].connected = true;
		
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

				case NEWPLAYER:
				{
					int newId = 0;
					int newPos = 0;
					int size = 0;
					pck >> size;
					
					for (unsigned int i = 0; i < size; i++) {
						pck >> aPlayers[i].ID >> aPlayers[i].pos.x >> aPlayers[i].pos.y;
						aPlayers[i].connected = true;
					}

					if (size == 4) 
						initPlay = true;
											
				}
				break;

				case CONTADOR:
				{
					strEP += "  ";
					pck >> strEP;

					empezarPartida = true;
					stclock.restart();
				}
				break;

				case MOVE:
				{
					int size = 0;
					pck >> size;

					switch (myId) {
						case 1:
							if (size == 1)		pck >> aPlayers[0].pos.x >> aPlayers[0].pos.y;
							else if (size == 2) pck >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[1].pos.x >> aPlayers[1].pos.y;
							else if (size == 3) pck >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[1].pos.x >> aPlayers[1].pos.y >> aPlayers[2].pos.x >> aPlayers[2].pos.y;
							else if (size == 4) pck >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[1].pos.x >> aPlayers[1].pos.y >> aPlayers[2].pos.x >> aPlayers[2].pos.y >> aPlayers[3].pos.x >> aPlayers[3].pos.y;
							break;

						case 2:
							if (size == 1)		pck >> aPlayers[1].pos.x >> aPlayers[1].pos.y;
							else if (size == 2) pck >> aPlayers[1].pos.x >> aPlayers[1].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y;
							else if (size == 3) pck >> aPlayers[1].pos.x >> aPlayers[1].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[2].pos.x >> aPlayers[2].pos.y;
							else if (size == 4) pck >> aPlayers[1].pos.x >> aPlayers[1].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[2].pos.x >> aPlayers[2].pos.y >> aPlayers[3].pos.x >> aPlayers[3].pos.y;
							break;

						case 3:
							if (size == 1)		pck >> aPlayers[2].pos.x >> aPlayers[2].pos.y;
							else if (size == 2) pck >> aPlayers[2].pos.x >> aPlayers[2].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y;
							else if (size == 3) pck >> aPlayers[2].pos.x >> aPlayers[2].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[1].pos.x >> aPlayers[1].pos.y;
							else if (size == 4) pck >> aPlayers[2].pos.x >> aPlayers[2].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[1].pos.x >> aPlayers[1].pos.y >> aPlayers[3].pos.x >> aPlayers[3].pos.y;
							break;

						case 4:
							if (size == 1)		pck >> aPlayers[3].pos.x >> aPlayers[3].pos.y;
							else if (size == 2) pck >> aPlayers[3].pos.x >> aPlayers[3].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y;
							else if (size == 3) pck >> aPlayers[3].pos.x >> aPlayers[3].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[1].pos.x >> aPlayers[1].pos.y;
							else if (size == 4) pck >> aPlayers[3].pos.x >> aPlayers[3].pos.y >> aPlayers[0].pos.x >> aPlayers[0].pos.y >> aPlayers[1].pos.x >> aPlayers[1].pos.y >> aPlayers[2].pos.x >> aPlayers[2].pos.y;
							break;

						default:
							break;
						
					}	
				}
				break;
				case ACK:
				{
					//packetes criticos!!
				}
				break;
				case PING:
					int id;
					int size;
					pck >> size >> id;
					std::cout << "Se ha desconectado el jugador " << id << "!" << std::endl;
					aPlayers[id-1].connected = false;
					clDisconnected.restart();
					disconnected = true;
				break;

				case RECEIVECOIN:
				{
					pck >> coin_pos.x >> coin_pos.y;
					coinPositioning = true;
				}
				break;

				case FINDEPARTIDA:
				{
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

		if (disconnected)
		{
			if (!aPlayers[myId- 1].connected) {
				std::cout << "Disconnected" << std::endl;
				sf::Font font;
				std::string pathFont = "arial.ttf";
				if (!font.loadFromFile(pathFont))
					std::cout << "No se pudo cargar la fuente" << std::endl;

				sf::Text text("  Disconnected player" + aPlayers[myId - 1].ID, font);
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

			switch (myId) {

				case 1:
					//if (!aPlayers[0].connected) DrawPlayer(window, sf::Color::White, aPlayers[0].pos);
					if (aPlayers[0].connected) DrawPlayer(window, sf::Color::Blue, aPlayers[0].pos);
					
					if (aPlayers.size() > 1 && aPlayers[1].connected) DrawPlayer(window, sf::Color::Green, aPlayers[1].pos);
					if (aPlayers.size() > 2 && aPlayers[2].connected) DrawPlayer(window, sf::Color::Red, aPlayers[2].pos);
					if (aPlayers.size() > 3 && aPlayers[3].connected) DrawPlayer(window, sf::Color::Yellow, aPlayers[3].pos);
				break;

				case 2:
					//if (!aPlayers[1].connected) DrawPlayer(window, sf::Color::White, aPlayers[1].pos);
					if (aPlayers[1].connected) DrawPlayer(window, sf::Color::Green, aPlayers[1].pos);

					if (aPlayers.size() > 1 && aPlayers[0].connected) DrawPlayer(window, sf::Color::Blue, aPlayers[0].pos);
					if (aPlayers.size() > 2 && aPlayers[2].connected) DrawPlayer(window, sf::Color::Red, aPlayers[2].pos);
					if (aPlayers.size() > 3 && aPlayers[3].connected) DrawPlayer(window, sf::Color::Yellow, aPlayers[3].pos);
				break;

				case 3:
					//if (!aPlayers[2].connected) DrawPlayer(window, sf::Color::White, aPlayers[2].pos);
					if (aPlayers[2].connected) DrawPlayer(window, sf::Color::Red, aPlayers[2].pos);
					
					if (aPlayers.size() > 1 && aPlayers[0].connected) DrawPlayer(window, sf::Color::Blue, aPlayers[0].pos);
					if (aPlayers.size() > 2 && aPlayers[1].connected) DrawPlayer(window, sf::Color::Green, aPlayers[1].pos);
					if (aPlayers.size() > 3 && aPlayers[3].connected) DrawPlayer(window, sf::Color::Yellow, aPlayers[3].pos);
				break;

				case 4:
					//if (!aPlayers[3].connected) DrawPlayer(window, sf::Color::White, aPlayers[3].pos);
					if (aPlayers[3].connected) DrawPlayer(window, sf::Color::Yellow, aPlayers[3].pos);
					
					if (aPlayers.size() > 1 && aPlayers[0].connected) DrawPlayer(window, sf::Color::Blue, aPlayers[0].pos);
					if (aPlayers.size() > 2 && aPlayers[1].connected) DrawPlayer(window, sf::Color::Green, aPlayers[1].pos);
					if (aPlayers.size() > 3 && aPlayers[2].connected) DrawPlayer(window, sf::Color::Red, aPlayers[2].pos);
				break;

				default:
				break;
			}
		}
	
		if (empezarPartida)
			DrawTextEP(window, stclock);

		// Si el jugador es mou li enviem cada 200ms la llista amb tota la acumulació (Acumulem cada cop que fem apretem una tecla)
		if (empezarPartida && clockMove.getElapsedTime().asMilliseconds() >= 200 && (deltax != 0 || deltay != 0)) {

			if (aPlayers[myId - 1].pos.x - coin_pos.x < 1.f && aPlayers[myId - 1].pos.y - coin_pos.y < 1.f && coinPositioning) {
				sf::Packet pa;
				enum PacketType enumCoin = PacketType::GETCOIN;
				pa << enumCoin << myId;
				sock.send(pa, IP_SERVER, PORT_SERVER);
				aPlayers[myId - 1].score++;
				std::cout << "Has puntuado! Tienes un total de " << aPlayers[myId - 1].score << " puntos!" << std::endl;
				coinPositioning = false;
			}

			int prevX = aPlayers[myId - 1].pos.x;
			int prevY = aPlayers[myId - 1].pos.y;
			idmove++;

			std::cout << "El jugador " << myId << " se ha movido! Delta Pos: (" << deltax << ", " << deltay << ") PosReal: (" << aPlayers[myId - 1].pos.x << ", " << aPlayers[myId - 1].pos.y << ")" << std::endl;

			Accum accum(myId, idmove, deltax, deltay, aPlayers[myId-1].pos.x, aPlayers[myId-1].pos.y);
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

		//Ganador
		if (winner) {
			sf::Font font;
			std::string pathFont = "arial.ttf";
			if (!font.loadFromFile(pathFont))
				std::cout << "No se pudo cargar la fuente" << std::endl;
			std::string ss = "                         El jugador " + std::to_string(idWinner) + " ha ganado la partida!";
			sf::Text text(ss.c_str() + aPlayers[myId - 1].ID, font);
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

	for (unsigned int i = 0; i < 4; i++) {
		Player newPlayer;
		newPlayer.connected = false;
		aPlayers.push_back(newPlayer);
	}

	sock.setBlocking(false);
	DibujaSFML();

	return 0;
}