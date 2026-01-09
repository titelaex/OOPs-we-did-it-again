#include <iostream>
#include <string>
#include <filesystem>
import Core.Game;
import Core.Board; 
import Models.Player;
import Core.Player;
import Core.GameState;
import Core.AIConfig;
import Core.TrainingLogger;
import Core.PlayerDecisionMaker;
int main()
{
    std::string username;
	auto& gameState = Core::GameState::getInstance();
	std::cout << "==============================================\n";
	std::cout << "   7 WONDERS DUEL - Britney vs Spears AI\n";
	std::cout << "==============================================\n\n";
	std::cout << "Choose game mode:\n";
	std::cout << "[1] Human vs Human\n";
	std::cout << "[2] Human vs AI\n";
	std::cout << "[3] AI vs AI (Training Mode)\n";
	std::cout << "[4] Human with AI Suggestions\n";
	std::cout << "Choice: ";
	int mode = 1;
	std::cin >> mode;
	std::cin.ignore();
	Core::IPlayerDecisionMaker* p1Decisions = nullptr;
	Core::IPlayerDecisionMaker* p2Decisions = nullptr;
	Core::Playstyle p1Playstyle = Core::Playstyle::BRITNEY;
	Core::Playstyle p2Playstyle = Core::Playstyle::BRITNEY;
	bool trainingMode = false;
	if (mode == 2) {
		std::cout << "\n=== HUMAN VS AI MODE ===\n";
		std::cout << "Enter your username: ";
		std::getline(std::cin, username);
		gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, username);
		gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, "AI_Opponent");
		std::cout << "\nChoose AI playstyle:\n";
		std::cout << "[1] Britney (Peaceful/Long-game)\n";
		std::cout << "[2] Spears (Aggressive/Military)\n";
		std::cout << "Choice: ";
		int aiStyle = 1;
		std::cin >> aiStyle;
		std::cin.ignore();
		p2Playstyle = (aiStyle == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
		p1Decisions = new Core::HumanDecisionMaker();
		p2Decisions = new Core::MCTSDecisionMaker(p2Playstyle, 1000, 1.414, 20);
		std::cout << "\n>> You are Player 1\n";
		std::cout << ">> AI is Player 2 playing as: " << Core::playstyleToString(p2Playstyle) << "\n\n";
	}
	else if (mode == 3) {
		std::cout << "\n=== AI VS AI TRAINING MODE ===\n";
		trainingMode = true;
		std::cout << "Select playstyle for AI Player 1:\n";
		std::cout << "[1] Britney (Peaceful/Long-game)\n";
		std::cout << "[2] Spears (Aggressive/Military)\n";
		std::cout << "Choice: ";
		int p1Style = 1;
		std::cin >> p1Style;
		std::cout << "Select playstyle for AI Player 2:\n";
		std::cout << "[1] Britney (Peaceful/Long-game)\n";
		std::cout << "[2] Spears (Aggressive/Military)\n";
		std::cout << "Choice: ";
		int p2Style = 1;
		std::cin >> p2Style;
		std::cin.ignore();
		p1Playstyle = (p1Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
		p2Playstyle = (p2Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
		gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, "AI_P1");
		gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, "AI_P2");
		p1Decisions = new Core::MCTSDecisionMaker(p1Playstyle, 1000, 1.414, 20);
		p2Decisions = new Core::MCTSDecisionMaker(p2Playstyle, 1000, 1.414, 20);
		std::cout << "\n>> AI Player 1 is playing as: " << Core::playstyleToString(p1Playstyle) << "\n";
		std::cout << ">> AI Player 2 is playing as: " << Core::playstyleToString(p2Playstyle) << "\n";
		std::cout << ">> Training data will be saved after the game.\n\n";
	}
	else if (mode == 4) {
		std::cout << "\n=== HUMAN WITH AI SUGGESTIONS ===\n";
		std::cout << "Enter Player 1 username: ";
		std::getline(std::cin, username);
		gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, username);
		std::cout << "Enter Player 2 username: ";
		std::getline(std::cin, username);
		gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, username);
		std::cout << "\nChoose suggestion style for Player 1:\n";
		std::cout << "[1] Britney (Peaceful/VP-focused suggestions)\n";
		std::cout << "[2] Spears (Aggressive/Military suggestions)\n";
		std::cout << "Choice: ";
		int p1Style = 1;
		std::cin >> p1Style;
		std::cout << "\nChoose suggestion style for Player 2:\n";
		std::cout << "[1] Britney (Peaceful/VP-focused suggestions)\n";
		std::cout << "[2] Spears (Aggressive/Military suggestions)\n";
		std::cout << "Choice: ";
		int p2Style = 1;
		std::cin >> p2Style;
		std::cin.ignore();
		p1Playstyle = (p1Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
		p2Playstyle = (p2Style == 1) ? Core::Playstyle::BRITNEY : Core::Playstyle::SPEARS;
		p1Decisions = new Core::HumanAssistedDecisionMaker(p1Playstyle, 500);
		p2Decisions = new Core::HumanAssistedDecisionMaker(p2Playstyle, 500);
		std::cout << "\n>> Player 1 gets " << Core::playstyleToString(p1Playstyle) << " suggestions\n";
		std::cout << ">> Player 2 gets " << Core::playstyleToString(p2Playstyle) << " suggestions\n\n";
	}
	else {
		std::cout << "\n=== HUMAN VS HUMAN MODE ===\n";
		std::cout << "Enter your username (p1): ";
		std::getline(std::cin, username);
		gameState.GetPlayer1()->m_player = std::make_unique<Models::Player>(1, username);
		std::cout << "Enter your username (p2): ";
		std::getline(std::cin, username);
		gameState.GetPlayer2()->m_player = std::make_unique<Models::Player>(2, username);
		p1Decisions = new Core::HumanDecisionMaker();
		p2Decisions = new Core::HumanDecisionMaker();
	}
	Core::TrainingLogger logger;
	if (trainingMode) {
		logger.startGame(p1Playstyle, p2Playstyle);
	}
    std::cout << "\nStarting preparation...\n";
    Core::Game::preparation();
    std::cout << "\n=== Wonder Selection ===\n";
    auto p1Ptr = gameState.GetPlayer1();
    auto p2Ptr = gameState.GetPlayer2();
    Core::Game::wonderSelection(p1Ptr, p2Ptr, p1Decisions, p2Decisions);
    std::cout << "Preparation finished.\n";
    Core::Board::getInstance().displayEntireBoard();
	std::cout << "\nSaving game state to gamestate.csv...\n";
	gameState.saveGameState("gamestate.csv");
	std::cout << "Game state saved.\n";
	Core::Player& p1 = *gameState.GetPlayer1();
	Core::Player& p2 = *gameState.GetPlayer2();
	std::cout << "\n=== Starting Phase I (Age I) ===\n";
	Core::Game::phaseI(p1, p2, p1Decisions, p2Decisions, trainingMode ? &logger : nullptr);
	std::cout << "\n=== Starting Phase II (Age II) ===\n";
	Core::Game::phaseII(p1, p2, p1Decisions, p2Decisions, trainingMode ? &logger : nullptr);
	std::cout << "\n=== Starting Phase III (Age III) ===\n";
	Core::Game::phaseIII(p1, p2, p1Decisions, p2Decisions, trainingMode ? &logger : nullptr);
	std::cout << "\n=== Final Board State ===\n";
	Core::Board::getInstance().displayEntireBoard();
	if (trainingMode) {
		std::cout << "\n=== Saving Training Data ===\n";
		auto* m1 = p1.m_player.get();
		auto* m2 = p2.m_player.get();
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
		std::string summaryFile = "TrainingData/" + playstyleDir + "/summary.csv";
		logger.saveGameToCSV(gameFile);
		logger.saveSummaryToCSV(summaryFile, true);
		std::cout << "Game data saved to: " << gameFile << "\n";
		std::cout << "Summary updated in: " << summaryFile << "\n";
	}
	delete p1Decisions;
	delete p2Decisions;
    return 0;
}
