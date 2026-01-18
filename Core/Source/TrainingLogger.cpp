#include <ctime>
module Core.TrainingLogger;
import <string>;
import <vector>;
import <fstream>;
import <sstream>;
import <iomanip>;
import <chrono>;
import <memory>;
import Core.AIConfig;
import Core.MCTS;
namespace Core {
TrainingLogger::TrainingLogger()
    : m_gameStarted(false)
{
    m_sessionId = generateSessionId();
}
void TrainingLogger::startGame(Playstyle p1Style, Playstyle p2Style) {
    reset();
    m_gameStarted = true;
    m_outcome.p1Playstyle = p1Style;
    m_outcome.p2Playstyle = p2Style;
    m_outcome.timestamp = generateTimestamp();
}
void TrainingLogger::logTurn(const TurnRecord& turn) {
    if (!m_gameStarted) return;
    m_turns.push_back(turn);
}
void TrainingLogger::endGame(int winner, const std::string& victoryType, uint32_t p1Score, uint32_t p2Score) {
    if (!m_gameStarted) return;
    m_outcome.winner = winner;
    m_outcome.victoryType = victoryType;
    m_outcome.totalTurns = static_cast<int>(m_turns.size());
    m_outcome.p1FinalScore = p1Score;
    m_outcome.p2FinalScore = p2Score;
    m_gameStarted = false;
}
void TrainingLogger::saveGameToCSV(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }
    file << "Turn,CurrentPlayer,Phase,MilitaryPos,"
         << "P1_Coins,P1_Cards,P1_Wonders,P1_Blue,P1_Green,P1_Red,"
         << "P2_Coins,P2_Cards,P2_Wonders,P2_Blue,P2_Green,P2_Red,"
         << "ActionType,CardName,CardIndex,StateValue,ExpectedReward\n";
    for (const auto& turn : m_turns) {
        file << turn.turnNumber << ","
             << (turn.player1Turn ? "P1" : "P2") << ","
             << turn.currentPhase << ","
             << turn.militaryPosition << ","
             << static_cast<int>(turn.p1Coins) << ","
             << static_cast<int>(turn.p1Cards) << ","
             << static_cast<int>(turn.p1Wonders) << ","
             << static_cast<int>(turn.p1BlueCards) << ","
             << static_cast<int>(turn.p1GreenCards) << ","
             << static_cast<int>(turn.p1RedCards) << ","
             << static_cast<int>(turn.p2Coins) << ","
             << static_cast<int>(turn.p2Cards) << ","
             << static_cast<int>(turn.p2Wonders) << ","
             << static_cast<int>(turn.p2BlueCards) << ","
             << static_cast<int>(turn.p2GreenCards) << ","
             << static_cast<int>(turn.p2RedCards) << ","
             << turn.actionType << ","
             << turn.cardName << ","
             << turn.cardNodeIndex << ","
             << turn.stateValue << ","
             << turn.expectedReward << "\n";
    }
    file.close();
}
void TrainingLogger::saveSummaryToCSV(const std::string& filename, bool append) {
    bool fileExists = std::ifstream(filename).good();
    std::ofstream file;
    if (append && fileExists) {
        file.open(filename, std::ios::app);
    } else {
        file.open(filename);
        file << "Timestamp,P1_Playstyle,P2_Playstyle,Winner,VictoryType,"
             << "TotalTurns,P1_FinalScore,P2_FinalScore\n";
    }
    if (!file.is_open()) {
        return;
    }
    file << m_outcome.timestamp << ","
         << playstyleToString(m_outcome.p1Playstyle) << ","
         << playstyleToString(m_outcome.p2Playstyle) << ","
         << (m_outcome.winner == 0 ? "P1" : (m_outcome.winner == 1 ? "P2" : "Tie")) << ","
         << m_outcome.victoryType << ","
         << m_outcome.totalTurns << ","
         << m_outcome.p1FinalScore << ","
         << m_outcome.p2FinalScore << "\n";
    file.close();
}
std::string TrainingLogger::generateTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::ostringstream oss;
    oss << seconds.count();
    return oss.str();
}
std::string TrainingLogger::generateSessionId() const {
    return generateTimestamp();
}
void TrainingLogger::reset() {
    m_turns.clear();
    m_outcome = GameOutcome();
    m_gameStarted = false;
}
TurnRecord createTurnRecord(const MCTSGameState& state, const MCTSAction& action, 
                            int turnNum, double stateValue, double expectedReward) {
    TurnRecord record;
    record.turnNumber = turnNum;
    record.player1Turn = state.player1Turn;
    record.currentPhase = state.currentPhase;
    record.militaryPosition = state.militaryPosition;
    record.p1Coins = state.player1.coins;
    record.p1Cards = state.player1.cardCount;
    record.p1Wonders = state.player1.wonderCount;
    record.p1BlueCards = state.player1.blueCards;
    record.p1GreenCards = state.player1.greenCards;
    record.p1RedCards = state.player1.redCards;
    record.p2Coins = state.player2.coins;
    record.p2Cards = state.player2.cardCount;
    record.p2Wonders = state.player2.wonderCount;
    record.p2BlueCards = state.player2.blueCards;
    record.p2GreenCards = state.player2.greenCards;
    record.p2RedCards = state.player2.redCards;
    switch (action.actionType) {
        case 0: record.actionType = "build"; break;
        case 1: record.actionType = "sell"; break;
        case 2: record.actionType = "wonder"; break;
        default: record.actionType = "unknown"; break;
    }
    record.cardName = action.cardName;
    record.cardNodeIndex = static_cast<int>(action.cardNodeIndex);
    record.stateValue = stateValue;
    record.expectedReward = expectedReward;
    return record;
}
} 
