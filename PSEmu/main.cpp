#include "imgui-SFML.h"
#include <SFML\Graphics.hpp>
#include "Core\Bus.h"


int main() {
	sf::RenderWindow window(sf::VideoMode(640, 460), "PSEmu");

	sf::Image icon;
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);

	Bus bus;
	bus.load_bios("bios/SCPH1001.BIN");

	sf::Color clear_color = sf::Color::Black;
	sf::Clock dtClock;
	while (window.isOpen()) {
		sf::Event ev;
		while (window.pollEvent(ev)) {
			ImGui::SFML::ProcessEvent(ev);
			switch (ev.type) {
			case sf::Event::Closed: window.close();
			}
		}

		ImGui::SFML::Update(window, dtClock.restart());


		window.clear(clear_color);

		ImGui::SFML::Render(window);

		window.display();
	}

	ImGui::SFML::Shutdown();


	return 0;
}