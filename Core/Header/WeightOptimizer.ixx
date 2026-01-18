export module Core.WeightOptimizer;
import <string>;
import <vector>;
import <unordered_map>;
import <memory>;
import Core.AIConfig;
export namespace Core {
struct PlaystyleStats {
    int totalGames = 0;
    int wins = 0;
    int losses = 0;
    int ties = 0;
    double avgScore = 0.0;
    double avgTurns = 0.0;
    int buildActions = 0;
    int sellActions = 0;
    int wonderActions = 0;
    int militaryVictories = 0;
    int scientificVictories = 0;
    int civilianVictories = 0;
    double winRate() const {
        return totalGames > 0 ? (static_cast<double>(wins) / totalGames) : 0.0;
    }
};
struct GameData {
    std::string timestamp;
    Playstyle p1Playstyle;
    Playstyle p2Playstyle;
    std::string winner; 
    std::string victoryType;
    int totalTurns;
    int p1FinalScore;
    int p2FinalScore;
};
struct TurnData {
    int turnNumber;
    std::string currentPlayer;
    int phase;
    int militaryPos;
    int p1Coins;
    int p1Cards;
    int p1Wonders;
    int p2Coins;
    int p2Cards;
    int p2Wonders;
    std::string actionType;
    std::string cardName;
    int cardIndex;
    double stateValue;
    double expectedReward;
};
export class WeightOptimizer {
private:
    std::unordered_map<Playstyle, PlaystyleStats> m_stats;
    std::vector<GameData> m_gameHistory;
    std::unordered_map<Playstyle, AIWeights> m_weights;
    double m_learningRate = 0.1;
    double m_momentum = 0.9;
public:
    WeightOptimizer();
    bool loadSummaryData(const std::string& csvPath);
    bool loadGameData(const std::string& csvPath, std::vector<TurnData>& turns);
    void analyzePerformance();
    void printStatistics(Playstyle style) const;
    void printComparison(Playstyle style1, Playstyle style2) const;
    void optimizeWeights(Playstyle style);
    void adjustWeightBasedOnPerformance(Playstyle style, const PlaystyleStats& stats);
    bool saveWeights(const std::string& filename, Playstyle style) const;
    bool loadWeights(const std::string& filename, Playstyle style);
    const AIWeights& getWeights(Playstyle style) const;
    const PlaystyleStats& getStats(Playstyle style) const;
private:
    void updateStats(const GameData& game);
    GameData parseGameRecord(const std::string& line);
    TurnData parseTurnRecord(const std::string& line);
    void increaseAggressiveness(AIWeights& weights, double factor);
    void increaseDefensiveness(AIWeights& weights, double factor);
    void balanceWeights(AIWeights& weights);
    Playstyle parsePlaystyle(const std::string& str) const;
};
export double calculateWeightAdjustment(double currentWeight, double performance, double learningRate);
} 
