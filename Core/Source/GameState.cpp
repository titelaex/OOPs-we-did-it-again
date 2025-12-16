module GameState;

import Core.Board;
import Core.Player;

import <string>;
import <fstream>;
import <iostream>;

export namespace Core
{
	void GameState::loadGameState(const std::string& filename)
	{

	}

	void GameState::saveGameState(const std::string& filename)
	{
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "Error: Could not open file " << filename << " for writing.\n";
			return;
		}

		outFile << m_board;
		outFile << m_player1;
		outFile << m_player2;
	}
} 

Core::GameState::GameState() : m_board(Core::Board::getInstance()) {}