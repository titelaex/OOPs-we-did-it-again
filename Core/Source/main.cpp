#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>
import Core.Game;
import Core.Board; 
import Models.Player;
import Core.Player;
import Core.GameState;
import Core.AIConfig;
import Core.TrainingLogger;
import Core.PlayerDecisionMaker;
import Core.GameMetadata;
import Core.PlayerNameValidator;

// Helper function to check if saved game exists
bool hasSavedGame(const std::string& filename) {
	return std::filesystem::exists(filename);
}

int main() {
	Core::GameState& gameState = Core::GameState::getInstance();
	
	bool continueGame = false;
	bool trainingMode = false;
	Core::Playstyle p1Playstyle = Core::Playstyle::BRITNEY;
	Core::Playstyle p2Playstyle = Core::Playstyle::BRITNEY;
	
	Core::IPlayerDecisionMaker* p1Decisions = nullptr;
	Core::IPlayerDecisionMaker* p2Decisions = nullptr;
	
	auto p1 = gameState.GetPlayer1();
	auto p2 = gameState.GetPlayer2();
	
	// Initialize game (game mode selection, player setup, wonder selection, etc.)
	Core::Game::initGame(continueGame, trainingMode, p1, p2, p1Decisions, p2Decisions, p1Playstyle, p2Playstyle);
	
	Core::TrainingLogger logger;
	if (trainingMode) {
		logger.startGame(p1Playstyle, p2Playstyle);
	}
	
	// Get player references for game phases (whether new or continued game)
	Core::Player& player1 = *p1;
	Core::Player& player2 = *p2;
	
	std::cout << "\n=== Starting Phase I (Age I) ===\n";
	Core::Game::phaseI(player1, player2, p1Decisions, p2Decisions, trainingMode ? &logger : nullptr);
	std::cout << "\n=== Starting Phase II (Age II) ===\n";
	Core::Game::phaseII(player1, player2, p1Decisions, p2Decisions, trainingMode ? &logger : nullptr);
	std::cout << "\n=== Starting Phase III (Age III) ===\n";
	Core::Game::phaseIII(player1, player2, p1Decisions, p2Decisions, trainingMode ? &logger : nullptr);
	std::cout << "\n=== Final Board State ===\n";
	Core::Board::getInstance().displayEntireBoard();
	
	// Determine winner and display results
	auto* m1 = player1.m_player.get();
	auto* m2 = player2.m_player.get();
	uint32_t score1 = 0, score2 = 0;
	
	if (m1) {
		const auto& pts1 = m1->getPoints();
		score1 = pts1.m_militaryVictoryPoints + pts1.m_buildingVictoryPoints + 
			         pts1.m_wonderVictoryPoints + pts1.m_progressVictoryPoints +
			         m1->totalCoins(m1->getRemainingCoins()) / 3;
	}
	if (m2) {
		const auto& pts2 = m2->getPoints();
		score2 = pts2.m_militaryVictoryPoints + pts2.m_buildingVictoryPoints + 
			         pts2.m_wonderVictoryPoints + pts2.m_progressVictoryPoints +
			         m2->totalCoins(m2->getRemainingCoins()) / 3;
	}
	
	// Show winner
	if (!trainingMode && score1 != score2) {
		if (score1 > score2) {
			std::cout << "\n[STATS] Player 1 wins!\n";
		} else {
			std::cout << "\n[STATS] Player 2 wins!\n";
		}
	}
	
	if (trainingMode) {
		std::cout << "\n=== Saving Training Data ===\n";
		int winner = (score1 > score2) ? 0 : (score2 > score1) ? 1 : 2;
		std::string victoryType = "Civilian";
		logger.endGame(winner, victoryType, score1, score2);
		std::string timestamp = logger.generateTimestamp();
		std::string playstyleDir = (p1Playstyle == Core::Playstyle::BRITNEY && p2Playstyle == Core::Playstyle::BRITNEY) ? "Britney" :
		                            (p1Playstyle == Core::Playstyle::SPEARS && p2Playstyle == Core::Playstyle::SPEARS) ? "Spears" : "Mixed";
		try {
			std::filesystem::create_directories("TrainingData/" + playstyleDir);
			std::cout << "Created directory: TrainingData/" << playstyleDir << "\n";
		} catch (const std::exception& e) {
			std::cerr << "Error creating directories: " << e.what() << "\n";
		}
		std::string gameFile = "TrainingData/" + playstyleDir + "/game_" + timestamp + ".csv";
		std::string summaryFile = "TrainingData/" + playstyleDir + "/summary_" + timestamp + ".csv";
		logger.saveGameToCSV(gameFile);
		logger.saveSummaryToCSV(summaryFile, true);
		std::cout << "Training data saved to:\n  " << gameFile << "\n  " << summaryFile << "\n";
	}
	
	// Cleanup decision makers
	if (!continueGame) {
		if (p1Decisions && dynamic_cast<Core::HumanAssistedDecisionMaker*>(p1Decisions)) {
			delete p1Decisions;
		} else if (p1Decisions && dynamic_cast<Core::MCTSDecisionMaker*>(p1Decisions)) {
			delete p1Decisions;
		}
		
		if (p2Decisions && dynamic_cast<Core::MCTSDecisionMaker*>(p2Decisions)) {
			delete p2Decisions;
		} else if (p2Decisions && dynamic_cast<Core::HumanAssistedDecisionMaker*>(p2Decisions)) {
			delete p2Decisions;
		}
	}
	
	std::cout << "\n=== Game Complete ===\n";
	return 0;
}
