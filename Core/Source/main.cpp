#include <iostream>
#include <string>

import Core.Game;
import Core.Board; 
import Models.Player;
import Core.Player;


int main()
{
    std::string username;

	std::cout << "Enter your username (p1): ";
	std::getline(std::cin, username);
	Models::Player player1(1, username);
    Core::Player p1;
	p1.m_player = std::make_unique<Models::Player>(std::move(player1));

	std::cout << "Enter your username (p2): ";
	std::getline(std::cin, username);
	Models::Player player2(2, username);
	Core::Player p2;
	p2.m_player = std::make_unique<Models::Player>(std::move(player2));

    std::cout << "Starting preparation test...\n";
    Core::preparation();
    std::cout << "Preparation finished.\n";
    Core::Board::getInstance().displayEntireBoard();

    return 0;
}
