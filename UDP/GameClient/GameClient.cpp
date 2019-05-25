#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <Player.h>
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

// Variables
sf::UdpSocket sock;
std::vector<Player> aPlayers;
std::list<Accum> aAccum;
sf::Vector2f ballPos;
std::string strEP;
int idmove = 0, deltax = 0, deltay = 0, idWinner = 0, speed = 2;
bool hello = false, initPlay = true, disconnected = false, empezarPartida = false, disappearText,
winner = false, ballPositioning = true, activarPerdida;

// Funciones
void DibujaSFML();
void DrawBall(sf::RenderWindow& window, sf::Vector2f pos);
void DrawPlayer(sf::RenderWindow& window, sf::Color color, sf::Vector2i pos, sf::Vector2i size);
void DrawTextEP(sf::RenderWindow& window, sf::Clock clock);
static float GetRandomFloat();

// MAIN
int main() {

	sock.setBlocking(false);
	DibujaSFML();

	return 0;
}

void DibujaSFML() {
	sf::Clock clCount;
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

				if (initPlay && aPlayers[0].connected && !aPlayers[0].dead) {

					if (event.key.code == sf::Keyboard::Space)
						speed = 4;

					if (event.key.code == sf::Keyboard::Left) {
						if (aPlayers[0].pos.x - speed >= 0) {
							deltax -= speed;
							aPlayers[0].pos.x -= speed;
						}
					}
					if (event.key.code == sf::Keyboard::Right) {
						if (aPlayers[0].pos.x + speed <= 740) {
							deltax += speed;
							aPlayers[0].pos.x += speed;
						}
					}
					if (event.key.code == sf::Keyboard::Up) {
						if (aPlayers[0].pos.y - speed >= 0) {
							deltay -= speed;
							aPlayers[0].pos.y -= speed;
						}
					}
					if (event.key.code == sf::Keyboard::Down) {
						if (aPlayers[0].pos.y + speed <= 540) {
							deltay += speed;
							aPlayers[0].pos.y += speed;
						}
					}
				}
				break;

			case sf::Event::KeyReleased:

				if (initPlay && aPlayers[0].connected) {

					if (event.key.code == sf::Keyboard::Space) 
						speed = 2;
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

					int myId = 0;
					int myPosX = 0;
					int myPosY = 0;
					int size = 0;

					pck >> myId >> myPosX >> myPosY >> ballPos.x >> ballPos.y >> size;

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
							for (int i = 0; i < size - 1; i++) {
								Player newPlayer;
								pck >> newPlayer.ID >> newPlayer.pos.x >> newPlayer.pos.y;
								newPlayer.connected = true;
								newPlayer.color = sf::Color::Red;
								aPlayers.push_back(newPlayer);
							}
						}

						std::cout << "Mensaje del servidor: WELCOME!" << std::endl;
						std::cout << "Soy el jugador " << myId << " con pos (" << myPosX << ", " << myPosY << ")" << std::endl;
						window.setTitle("Slither.io - Jugador " + std::to_string(myId) + " Score: " + std::to_string(myPlayer.score));
						hello = true;
					}

					//if (activarPerdida) {
					//	float rndPacketLoss = GetRandomFloat();
					//	if (rndPacketLoss < PERCENT_PACKETLOSS) {
					//		InputMemoryBitStream imbs(_message, bytesReceived);
					//		enum PacketType pt = PacketType::EMPTY;
					//		imbs.Read(&pt, 3);
					//		std::cout << rndPacketLoss << "Simulamos que se pierde msg de tipo " << pt << " - " << std::endl;
					//		return -1;
					//	}
					//}

				}
				break;

				case NEWPLAYER: {

					int id;
					int posx;
					int posy;

					pck >> id >> posx >> posy;

					Player newPlayer;
					newPlayer.ID = id;
					newPlayer.pos.x = posx;
					newPlayer.pos.y = posy;
					newPlayer.connected = true;
					newPlayer.color = sf::Color::Red;

					bool isNew = true;
					for (int i = 0; i < aPlayers.size(); i++) {
						if (newPlayer.ID == aPlayers[i].ID)
							isNew = false;
					}

					if (isNew)
						aPlayers.push_back(newPlayer);

					// Enviamos ACK
					sf::Packet newpck;
					enum PacketType enumack = PacketType::ACK;
					newpck << enumack << aPlayers[0].ID;
					sock.send(newpck, IP_SERVER, PORT_SERVER);
				}
				break;

				case CONTADOR: {

					strEP += "  ";
					pck >> strEP;

					empezarPartida = true;
					stclock.restart();
				}
				break;

				// Actualizar posiciones validas de los otros jugadores
				case MOVE: {
				
					bool valid;
					int size;
					int id;
					pck >> valid >> size >> id;

					// Si es valida la posición actualizamos el jugador que se ha movido
					// Pero si es valida, no actualizamos la pos del jugador local aPlayers[0] ya que por predicción ya se ha movido
					if (valid) {
						for (int i = 1; i < size; i++) {
							if (aPlayers[i].ID == id)
								pck >> aPlayers[i].pos.x >> aPlayers[i].pos.y;
						}
					}
					// Si no, no lo actualizamos y mostramos mensaje por pantalla
					else {
						int posx;
						int posy;
						pck >> posx >> posy;
						std::cout << "La pos (" << posx << ", " << posy << ") NO es valida!" << std::endl;
					}
				}
				break;

				// Confirmación desde servidor que la pos del jugador local es valida o no
				case ACKMOVE: {

					bool valid;
					int posx;
					int posy;
					pck >> valid >> posx >> posy;

					// Si la pos es valida no hacemos nada, ya que al usar predicción el jugador ya se haya en esa pos
					if (valid)
						std::cout << "La pos es valida!" << std::endl;
					// Si la pos NO es valida, devolvemos al jugador a la pos anterior
					else {
						std::cout << "La pos NO es valida! Vuelves a la anterior!" << std::endl;
						aPlayers[0].pos.x = posx;
						aPlayers[0].pos.y = posy;
					}
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

				case RECEIVEBALL: {
				
					if (!ballPositioning) {
						pck >> ballPos.x >> ballPos.y;
						std::cout << "BALL " << ballPos.x << " " << ballPos.y << std::endl;
						ballPositioning = true;
					}
				}
				break;

				case DIE: {

					int id1, id2;
					pck >> id1 >> id2;

					for (int i = 0; i < aPlayers.size(); i++) {
						if (aPlayers[i].ID == id1 || aPlayers[i].ID == id2) {
							aPlayers[i].dead = true;
						}
					}
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

				if (clDisconnected.getElapsedTime().asSeconds() >= 5) {
					disconnected = false;
					clDisconnected.restart();
				}
			}

		}

		// Fondo
		sf::Texture texture;
		texture.loadFromFile("bg.png");

		sf::Sprite sprite(texture);
		sprite.setScale(1.f, 1.f);
		sprite.setPosition(0.f, 0.f);
		window.draw(sprite);

		// Bola
		if (disappearText && !winner) {
			DrawBall(window, ballPos);
		}

		// Pintar jugadores
		if (aPlayers.size() > 0) {
			for (int i = 0; i < aPlayers.size(); i++) {
				if (aPlayers[i].connected && aPlayers[i].dead)
					DrawBall(window, sf::Vector2f(aPlayers[i].pos));
				else if (aPlayers[i].connected && !aPlayers[i].dead)
					DrawPlayer(window, aPlayers[i].color, sf::Vector2i(aPlayers[i].pos), sf::Vector2i(aPlayers[i].size));
			}
		}

		if (empezarPartida)
			DrawTextEP(window, stclock);

		// Si el jugador se mueve enviamos cada Xms una lista con toda la acumulación a servidor (acumulamos cada vez que se pulsa una tecla)
		if (empezarPartida && clockMove.getElapsedTime().asMilliseconds() >= 200 && (deltax != 0 || deltay != 0)) {

			if (abs((float)aPlayers[0].pos.x - ballPos.x) < 15.f && abs((float)aPlayers[0].pos.y - ballPos.y) < 15.f && (ballPositioning)) {
				sf::Packet pa;
				enum PacketType enumBall = PacketType::GETBALL;
				pa << enumBall << aPlayers[0].ID;
				sock.send(pa, IP_SERVER, PORT_SERVER);
				aPlayers[0].score++;
				window.setTitle("Slither.io - Jugador " + std::to_string(aPlayers[0].ID) + " Score: " + std::to_string(aPlayers[0].score));
				aPlayers[0].size.y += 20;
				std::cout << "Has puntuado! Tienes un total de " << aPlayers[0].score << " puntos!" << std::endl;
				ballPositioning = false;
			}

			int prevX = aPlayers[0].pos.x;
			int prevY = aPlayers[0].pos.y;
			idmove++;

			std::cout << "El jugador " << aPlayers[0].ID << " se ha movido! Delta Pos (" << deltax << ", " << deltay << ") PosReal (" << aPlayers[0].pos.x << ", " << aPlayers[0].pos.y << ")" << std::endl;

			Accum accum(aPlayers[0].ID, idmove, deltax, deltay, aPlayers[0].pos.x, aPlayers[0].pos.y);
			aAccum.push_back(accum);

			// MOVE
			sf::Packet pack = accum.AccumPacket();
			sock.send(pack, IP_SERVER, PORT_SERVER);

			deltax = 0;
			deltay = 0;
			clockMove.restart();

			// Mirar colisiones
			if (aPlayers.size() > 1 && !aPlayers[0].dead) {
				for (int i = 1; i < aPlayers.size(); i++) {
					if (!aPlayers[i].dead) {
						if (abs((float)aPlayers[0].pos.x - (float)aPlayers[i].pos.x) < 20.f && abs((float)aPlayers[0].pos.y - (float)aPlayers[i].pos.y) < 20.f) {
							aPlayers[0].dead = true;
							aPlayers[i].dead = true;
							sf::Packet newpck;
							enum PacketType enumack = PacketType::DIE;
							newpck << enumack << aPlayers[0].ID << aPlayers[i].ID;
							sock.send(newpck, IP_SERVER, PORT_SERVER);
						}
					}
				}
			}

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

// Sirve para dibujar bolitas
void DrawBall(sf::RenderWindow& window, sf::Vector2f pos) {
	sf::CircleShape ball(10);
	ball.setFillColor(sf::Color::White);
	ball.setPosition(pos);
	window.draw(ball);
}

// Sirve para ahorrar código a la hora de dibujar jugadores por pantalla
void DrawPlayer(sf::RenderWindow& window, sf::Color color, sf::Vector2i pos, sf::Vector2i size) {
	sf::RectangleShape rectAvatar(sf::Vector2f(size.x, size.y));
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
		text.setPosition(sf::Vector2f(50.f, 50.f));
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