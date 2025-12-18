#include <iostream>
#include <string>

import Core.Game;
import Core.Board; 
import Models.Player;
import Core.Player;
import GameState;


int main()
{
    std::string username;
	auto& gameState = Core::GameState::getInstance();

	std::cout << "Enter your username (p1): ";
	std::getline(std::cin, username);
	gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, username);

	std::cout << "Enter your username (p2): ";
	std::getline(std::cin, username);
	gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, username);

    std::cout << "Starting preparation test...\n";
    Core::preparation();
    std::cout << "Preparation finished.\n";
    Core::Board::getInstance().displayEntireBoard();

	std::cout << "Saving game state to gamestate.csv...\n";
	gameState.saveGameState("gamestate.csv");
	std::cout << "Game state saved.\n";

	/*std::cout << "loading game state from gamestate.csv...\n";
	gameState.loadGameState("gamestate.csv");
	std::cout << "Game state loaded.\n";
	Core::Board::getInstance().displayEntireBoard();*/

    return 0;
}
