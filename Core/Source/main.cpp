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

	// Ob?ine referin?e la juc?tori
	Core::Player& p1 = *gameState.GetPlayer1();
	Core::Player& p2 = *gameState.GetPlayer2();

	// Joac? Phase I (Age I)
	std::cout << "\n=== Starting Phase I (Age I) ===\n";
	Core::phaseI(p1, p2);

	// Joac? Phase II (Age II)
	std::cout << "\n=== Starting Phase II (Age II) ===\n";
	Core::phaseII(p1, p2);

	// Joac? Phase III (Age III) - include verificarea victoriei civile la final
	std::cout << "\n=== Starting Phase III (Age III) ===\n";
	Core::phaseIII(p1, p2);

	// Afi?eaz? starea final?
	std::cout << "\n=== Final Board State ===\n";
	Core::Board::getInstance().displayEntireBoard();

	/*std::cout << "loading game state from gamestate.csv...\n";
	gameState.loadGameState("gamestate.csv");
	std::cout << "Game state loaded.\n";
	Core::Board::getInstance().displayEntireBoard();*/

    return 0;
}
