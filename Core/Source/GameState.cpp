module Core.GameState;
import Core.Board;
import Core.Player;
import <string>;
import <fstream>;
import <iostream>;
import <memory>;
namespace Core
{
	void GameState::loadGameState(const std::string& filename)
	{
		std::ifstream inFile(filename);
		if (!inFile.is_open()) {
			std::cerr << "Error: Could not open file " << filename << " for reading.\n";
			return;
		}
		inFile >> m_board;
		if (m_player1) inFile >> *m_player1;
		if (m_player2) inFile >> *m_player2;
	}
	void GameState::saveGameState(const std::string& filename)
	{
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "Error: Could not open file " << filename << " for writing.\n";
			return;
		}
		outFile << m_board;
		if (m_player1) outFile << *m_player1;
		if (m_player2) outFile << *m_player2;
	}
} 
Core::GameState::GameState() : m_board(Core::Board::getInstance())
{
	m_player1 = std::make_shared<Core::Player>();
	m_player2 = std::make_shared<Core::Player>();
}