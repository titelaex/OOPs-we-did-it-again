module Core.WeightOptimizer;
import <string>;
import <vector>;
import <fstream>;
import <sstream>;
import <iostream>;
import <iomanip>;
import <algorithm>;
import <unordered_map>;
import Core.AIConfig;
#include <cmath>
namespace Core {
WeightOptimizer::WeightOptimizer() {
    m_weights[Playstyle::BRITNEY] = Core::getDefaultWeights(Playstyle::BRITNEY);
    m_weights[Playstyle::SPEARS] = Core::getDefaultWeights(Playstyle::SPEARS);
}
bool WeightOptimizer::loadSummaryData(const std::string& csvPath) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open: " << csvPath << "\n";
        return false;
    }
    std::string line;
    std::getline(file, line); 
    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        try {
            GameData game = parseGameRecord(line);
            m_gameHistory.push_back(game);
            updateStats(game);
            count++;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing line: " << e.what() << "\n";
        }
    }
    file.close();
    std::cout << "Loaded " << count << " games from " << csvPath << "\n";
    return count > 0;
}
bool WeightOptimizer::loadGameData(const std::string& csvPath, std::vector<TurnData>& turns) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        return false;
    }
    std::string line;
    std::getline(file, line); 
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        try {
            TurnData turn = parseTurnRecord(line);
            turns.push_back(turn);
        } catch (...) {
            continue;
        }
    }
    file.close();
    return !turns.empty();
}
GameData WeightOptimizer::parseGameRecord(const std::string& line) {
    GameData data;
    std::stringstream ss(line);
    std::string field;
    std::getline(ss, data.timestamp, ',');
    std::getline(ss, field, ',');
    data.p1Playstyle = parsePlaystyle(field);
    std::getline(ss, field, ',');
    data.p2Playstyle = parsePlaystyle(field);
    std::getline(ss, data.winner, ',');
    std::getline(ss, data.victoryType, ',');
    std::getline(ss, field, ',');
    data.totalTurns = std::stoi(field);
    std::getline(ss, field, ',');
    data.p1FinalScore = std::stoi(field);
    std::getline(ss, field, ',');
    data.p2FinalScore = std::stoi(field);
    return data;
}
TurnData WeightOptimizer::parseTurnRecord(const std::string& line) {
    TurnData data;
    std::stringstream ss(line);
    std::string field;
    std::getline(ss, field, ','); data.turnNumber = std::stoi(field);
    std::getline(ss, data.currentPlayer, ',');
    std::getline(ss, field, ','); data.phase = std::stoi(field);
    std::getline(ss, field, ','); data.militaryPos = std::stoi(field);
    std::getline(ss, field, ','); data.p1Coins = std::stoi(field);
    std::getline(ss, field, ','); data.p1Cards = std::stoi(field);
    std::getline(ss, field, ','); data.p1Wonders = std::stoi(field);
    std::getline(ss, field, ','); 
    std::getline(ss, field, ','); 
    std::getline(ss, field, ','); 
    std::getline(ss, field, ','); data.p2Coins = std::stoi(field);
    std::getline(ss, field, ','); data.p2Cards = std::stoi(field);
    std::getline(ss, field, ','); data.p2Wonders = std::stoi(field);
    std::getline(ss, field, ','); 
    std::getline(ss, field, ','); 
    std::getline(ss, field, ','); 
    std::getline(ss, data.actionType, ',');
    std::getline(ss, data.cardName, ',');
    std::getline(ss, field, ','); data.cardIndex = std::stoi(field);
    std::getline(ss, field, ','); data.stateValue = std::stod(field);
    std::getline(ss, field, ','); data.expectedReward = std::stod(field);
    return data;
}
void WeightOptimizer::updateStats(const GameData& game) {
    auto& p1Stats = m_stats[game.p1Playstyle];
    p1Stats.totalGames++;
    p1Stats.avgTurns = (p1Stats.avgTurns * (p1Stats.totalGames - 1) + game.totalTurns) / p1Stats.totalGames;
    p1Stats.avgScore = (p1Stats.avgScore * (p1Stats.totalGames - 1) + game.p1FinalScore) / p1Stats.totalGames;
    if (game.winner == "P1") {
        p1Stats.wins++;
        if (game.victoryType.find("Military") != std::string::npos) p1Stats.militaryVictories++;
        else if (game.victoryType.find("Scientific") != std::string::npos) p1Stats.scientificVictories++;
        else p1Stats.civilianVictories++;
    } else if (game.winner == "P2") {
        p1Stats.losses++;
    } else {
        p1Stats.ties++;
    }
    auto& p2Stats = m_stats[game.p2Playstyle];
    p2Stats.totalGames++;
    p2Stats.avgTurns = (p2Stats.avgTurns * (p2Stats.totalGames - 1) + game.totalTurns) / p2Stats.totalGames;
    p2Stats.avgScore = (p2Stats.avgScore * (p2Stats.totalGames - 1) + game.p2FinalScore) / p2Stats.totalGames;
    if (game.winner == "P2") {
        p2Stats.wins++;
        if (game.victoryType.find("Military") != std::string::npos) p2Stats.militaryVictories++;
        else if (game.victoryType.find("Scientific") != std::string::npos) p2Stats.scientificVictories++;
        else p2Stats.civilianVictories++;
    } else if (game.winner == "P1") {
        p2Stats.losses++;
    } else {
        p2Stats.ties++;
    }
}
void WeightOptimizer::analyzePerformance() {
    std::cout << "\n=== PERFORMANCE ANALYSIS ===\n\n";
    for (const auto& [style, stats] : m_stats) {
        if (stats.totalGames > 0) {
            printStatistics(style);
        }
    }
}
void WeightOptimizer::printStatistics(Playstyle style) const {
    auto it = m_stats.find(style);
    if (it == m_stats.end() || it->second.totalGames == 0) {
        std::cout << playstyleToString(style) << ": No data\n";
        return;
    }
    const auto& stats = it->second;
    std::cout << "=== " << playstyleToString(style) << " Statistics ===\n";
    std::cout << "Total Games: " << stats.totalGames << "\n";
    std::cout << "Win Rate: " << std::fixed << std::setprecision(1) 
              << (stats.winRate() * 100.0) << "%\n";
    std::cout << "Record: " << stats.wins << "W - " << stats.losses << "L - " << stats.ties << "T\n";
    std::cout << "Avg Score: " << std::fixed << std::setprecision(1) << stats.avgScore << "\n";
    std::cout << "Avg Game Length: " << std::fixed << std::setprecision(1) << stats.avgTurns << " turns\n";
    std::cout << "\nVictory Types:\n";
    std::cout << "  Military: " << stats.militaryVictories << "\n";
    std::cout << "  Scientific: " << stats.scientificVictories << "\n";
    std::cout << "  Civilian: " << stats.civilianVictories << "\n";
    std::cout << "\n";
}
void WeightOptimizer::printComparison(Playstyle style1, Playstyle style2) const {
    auto it1 = m_stats.find(style1);
    auto it2 = m_stats.find(style2);
    if (it1 == m_stats.end() || it2 == m_stats.end()) {
        std::cout << "Insufficient data for comparison\n";
        return;
    }
    const auto& stats1 = it1->second;
    const auto& stats2 = it2->second;
    std::cout << "\n=== " << playstyleToString(style1) << " vs " << playstyleToString(style2) << " ===\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Win Rate: " << (stats1.winRate() * 100.0) << "% vs " << (stats2.winRate() * 100.0) << "%\n";
    std::cout << "Avg Score: " << stats1.avgScore << " vs " << stats2.avgScore << "\n";
    std::cout << "Avg Turns: " << stats1.avgTurns << " vs " << stats2.avgTurns << "\n";
    std::cout << "\n";
}
void WeightOptimizer::optimizeWeights(Playstyle style) {
    auto it = m_stats.find(style);
    if (it == m_stats.end() || it->second.totalGames < 5) {
        std::cout << "Insufficient data to optimize " << playstyleToString(style) << " (need at least 5 games)\n";
        return;
    }
    const auto& stats = it->second;
    adjustWeightBasedOnPerformance(style, stats);
}
void WeightOptimizer::adjustWeightBasedOnPerformance(Playstyle style, const PlaystyleStats& stats) {
    auto& weights = m_weights[style];
    double winRate = stats.winRate();
    double performanceScore = winRate; 
    std::cout << "\n=== Optimizing " << playstyleToString(style) << " Weights ===\n";
    std::cout << "Current Performance: " << std::fixed << std::setprecision(1) << (winRate * 100.0) << "%\n";
    double adjustmentMagnitude = (winRate < 0.4) ? m_learningRate * 2.0 : 
                                  (winRate > 0.6) ? m_learningRate * 0.5 : m_learningRate;
    int totalVictories = stats.wins;
    if (totalVictories > 0) {
        double militaryRatio = static_cast<double>(stats.militaryVictories) / totalVictories;
        double scientificRatio = static_cast<double>(stats.scientificVictories) / totalVictories;
        double civilianRatio = static_cast<double>(stats.civilianVictories) / totalVictories;
        if (militaryRatio > 0.5) {
            weights.militaryPriority += adjustmentMagnitude * 0.5;
            weights.wonderMilitaryBonus += adjustmentMagnitude * 0.3;
            std::cout << "  ↑ Increased military focus (winning via military)\n";
        }
        if (scientificRatio > 0.5) {
            weights.sciencePriority += adjustmentMagnitude * 0.5;
            std::cout << "  ↑ Increased science focus\n";
        }
        if (civilianRatio > 0.5) {
            weights.victoryPointValue += adjustmentMagnitude * 0.5;
            weights.wonderVPBonus += adjustmentMagnitude * 0.3;
            std::cout << "  ↑ Increased VP focus\n";
        }
    }
    if (winRate < 0.4) {
        std::cout << "  ⚖ Rebalancing weights (poor performance)\n";
        balanceWeights(weights);
    }
    weights.normalizeWeights();
    std::cout << "Optimization complete!\n\n";
}
void WeightOptimizer::increaseAggressiveness(AIWeights& weights, double factor) {
    weights.militaryPriority *= (1.0 + factor);
    weights.wonderMilitaryBonus *= (1.0 + factor);
    weights.opponentDenial *= (1.0 + factor);
}
void WeightOptimizer::increaseDefensiveness(AIWeights& weights, double factor) {
    weights.resourceValue *= (1.0 + factor);
    weights.economyPriority *= (1.0 + factor);
    weights.wonderVPBonus *= (1.0 + factor);
}
void WeightOptimizer::balanceWeights(AIWeights& weights) {
    weights.militaryPriority = weights.militaryPriority * 0.8 + 1.0 * 0.2;
    weights.sciencePriority = weights.sciencePriority * 0.8 + 1.0 * 0.2;
    weights.economyPriority = weights.economyPriority * 0.8 + 1.0 * 0.2;
    weights.victoryPointValue = weights.victoryPointValue * 0.8 + 1.0 * 0.2;
}
bool WeightOptimizer::saveWeights(const std::string& filename, Playstyle style) const {
    auto it = m_weights.find(style);
    if (it == m_weights.end()) {
        return false;
    }
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    const auto& w = it->second;
    file << "# Optimized weights for " << playstyleToString(style) << "\n";
    file << "# Generated by WeightOptimizer\n\n";
    file << std::fixed << std::setprecision(4);
    file << "victoryPointValue=" << w.victoryPointValue << "\n";
    file << "militaryPriority=" << w.militaryPriority << "\n";
    file << "sciencePriority=" << w.sciencePriority << "\n";
    file << "economyPriority=" << w.economyPriority << "\n";
    file << "resourceValue=" << w.resourceValue << "\n";
    file << "coinValue=" << w.coinValue << "\n";
    file << "wonderVPBonus=" << w.wonderVPBonus << "\n";
    file << "wonderMilitaryBonus=" << w.wonderMilitaryBonus << "\n";
    file << "wonderEconomyBonus=" << w.wonderEconomyBonus << "\n";
    file << "opponentDenial=" << w.opponentDenial << "\n";
    file << "futureValueDiscount=" << w.futureValueDiscount << "\n";
    file.close();
    return true;
}
bool WeightOptimizer::loadWeights(const std::string& filename, Playstyle style) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    AIWeights weights;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        double value = std::stod(line.substr(pos + 1));
        if (key == "victoryPointValue") weights.victoryPointValue = value;
        else if (key == "militaryPriority") weights.militaryPriority = value;
        else if (key == "sciencePriority") weights.sciencePriority = value;
        else if (key == "economyPriority") weights.economyPriority = value;
        else if (key == "resourceValue") weights.resourceValue = value;
        else if (key == "coinValue") weights.coinValue = value;
        else if (key == "wonderVPBonus") weights.wonderVPBonus = value;
        else if (key == "wonderMilitaryBonus") weights.wonderMilitaryBonus = value;
        else if (key == "wonderEconomyBonus") weights.wonderEconomyBonus = value;
        else if (key == "opponentDenial") weights.opponentDenial = value;
        else if (key == "futureValueDiscount") weights.futureValueDiscount = value;
    }
    file.close();
    m_weights[style] = weights;
    return true;
}
const AIWeights& WeightOptimizer::getWeights(Playstyle style) const {
    auto it = m_weights.find(style);
    if (it != m_weights.end()) {
        return it->second;
    }
    return Core::getDefaultWeights(style);
}
const PlaystyleStats& WeightOptimizer::getStats(Playstyle style) const {
    static PlaystyleStats empty;
    auto it = m_stats.find(style);
    return (it != m_stats.end()) ? it->second : empty;
}
Playstyle WeightOptimizer::parsePlaystyle(const std::string& str) const {
    if (str == "Britney") return Playstyle::BRITNEY;
    if (str == "Spears") return Playstyle::SPEARS;
    return Playstyle::BRITNEY; 
}
double calculateWeightAdjustment(double currentWeight, double performance, double learningRate) {
    double delta = (performance - 0.5) * learningRate;
    return currentWeight + delta;
}
} 
