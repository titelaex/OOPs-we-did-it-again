module GameState;

import Core.Board;
import Core.Player;

import <string>;

export namespace Core
{
	void GameState::loadGameState(const std::string& filename)
	{

	}

	void GameState::saveGameState(const std::string& filename)
	{

	}
} 

Core::GameState::GameState() : m_board(Core::Board::getInstance()) {}