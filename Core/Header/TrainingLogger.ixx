export module Core.TrainingLogger;
import <string>;
import <vector>;
import <memory>;
import <fstream>;
import <chrono>;
import <sstream>;
import <iomanip>;
import Core.AIConfig;
export namespace Core {
    struct MCTSGameState;
    struct MCTSAction;
    struct TurnRecord {
        int turnNumber;
        bool player1Turn;
        int currentPhase;
        int militaryPosition;
        uint8_t p1Coins;
        uint8_t p1Cards;
        uint8_t p1Wonders;
        uint8_t p1BlueCards;
        uint8_t p1GreenCards;
        uint8_t p1RedCards;
        uint8_t p2Coins;
        uint8_t p2Cards;
        uint8_t p2Wonders;
        uint8_t p2BlueCards;
        uint8_t p2GreenCards;
        uint8_t p2RedCards;
        std::string actionType;     
        std::string cardName;
        int cardNodeIndex;
        double stateValue;          
        double expectedReward;      
        TurnRecord() : turnNumber(0), player1Turn(true), currentPhase(1), militaryPosition(9),
                       p1Coins(7), p1Cards(0), p1Wonders(0), p1BlueCards(0), p1GreenCards(0), p1RedCards(0),
                       p2Coins(7), p2Cards(0), p2Wonders(0), p2BlueCards(0), p2GreenCards(0), p2RedCards(0),
                       cardNodeIndex(-1), stateValue(0.0), expectedReward(0.0) {}
    };
    struct GameOutcome {
        int winner;                    
        std::string victoryType;       
        int totalTurns;
        uint32_t p1FinalScore;
        uint32_t p2FinalScore;
        Playstyle p1Playstyle;
        Playstyle p2Playstyle;
        std::string timestamp;
        GameOutcome() : winner(-1), totalTurns(0), p1FinalScore(0), p2FinalScore(0),
                        p1Playstyle(Playstyle::BRITNEY), p2Playstyle(Playstyle::BRITNEY) {}
    };
    class TrainingLogger {
    private:
        std::vector<TurnRecord> m_turns;
        GameOutcome m_outcome;
        std::string m_sessionId;
        bool m_gameStarted;
    public:
        TrainingLogger();
        void startGame(Playstyle p1Style, Playstyle p2Style);
        void logTurn(const TurnRecord& turn);
        void endGame(int winner, const std::string& victoryType, uint32_t p1Score, uint32_t p2Score);
        void saveGameToCSV(const std::string& filename);
        void saveSummaryToCSV(const std::string& filename, bool append = true);
        std::string generateTimestamp() const;
        std::string generateSessionId() const;
        const std::vector<TurnRecord>& getTurns() const { return m_turns; }
        const GameOutcome& getOutcome() const { return m_outcome; }
        bool isGameStarted() const { return m_gameStarted; }
        void reset();
    };
    TurnRecord createTurnRecord(const MCTSGameState& state, const MCTSAction& action, 
                                int turnNum, double stateValue, double expectedReward);
} 
