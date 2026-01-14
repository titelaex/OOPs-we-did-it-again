module Core.PlayerDecisionMaker;
import <vector>;
import <iostream>;
import <memory>;
import <random>;
import <algorithm>;
import <iomanip>;
import Core.MCTS;
import Core.Player;
import Core.Board;
import Core.GameState;
import Core.Node;
import Core.AIConfig;
import Models.Card;
import Models.AgeCard;
import Models.Wonder;
namespace Core {
size_t HumanDecisionMaker::selectCard(const std::vector<size_t>& available) {
    size_t choice = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter a valid number (0-" << (available.size() - 1) << "): ";
            continue;
        }
        if (choice >= available.size()) {
            std::cout << "ERROR: Invalid choice! Please enter a number between 0 and " << (available.size() - 1) << ": ";
            continue;
        }
        validInput = true;
    }
    return choice;
}
int HumanDecisionMaker::selectCardAction() {
    int action = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> action;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter 0, 1, or 2: ";
            continue;
        }
        if (action < 0 || action > 2) {
            std::cout << "ERROR: Invalid action! Please enter 0 (build), 1 (sell), or 2 (wonder): ";
            continue;
        }
        validInput = true;
    }
    return action;
}
size_t HumanDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
    size_t choice = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter a valid number (0-" << (candidates.size() - 1) << "): ";
            continue;
        }
        if (choice >= candidates.size()) {
            std::cout << "ERROR: Invalid wonder choice! Please enter a number between 0 and " << (candidates.size() - 1) << ": ";
            continue;
        }
        validInput = true;
    }
    return choice;
}
std::uint8_t HumanDecisionMaker::selectStartingPlayer() {
    int choice = 0;
    std::cin >> choice;
    return (choice == 0) ? 0 : 1;
}
MCTSDecisionMaker::MCTSDecisionMaker(Playstyle playstyle, int iterations, double explorationConstant, int maxDepth)
    : m_playstyle(playstyle)
    , m_iterations(iterations)
    , m_explorationConstant(explorationConstant)
    , m_maxDepth(maxDepth)
{
    m_mcts = std::make_unique<MCTS>(iterations, explorationConstant, maxDepth, playstyle);
    std::cout << "[AI] Initialized " << playstyleToString(playstyle) 
              << " AI (iterations=" << iterations << ", weights: ";
    AIWeights weights;
    std::string styleStr = playstyleToString(playstyle);
    std::string optimizedPath = "OptimizedWeights/" + styleStr + "_optimized.weights";
    if (weights.loadFromFile(optimizedPath)) {
        std::cout << "OPTIMIZED";
    } else {
        std::cout << "DEFAULT";
    }
    std::cout << ")\n";
}
MCTSDecisionMaker::~MCTSDecisionMaker() = default;
void MCTSDecisionMaker::setPlaystyle(Playstyle style) {
    m_playstyle = style;
    m_mcts->setPlaystyle(style);
}
void MCTSDecisionMaker::setIterations(int count) {
    m_iterations = count;
    m_mcts->setIterations(count);
}
void MCTSDecisionMaker::setExplorationConstant(double constant) {
    m_explorationConstant = constant;
    m_mcts->setExplorationConstant(constant);
}
void MCTSDecisionMaker::setMaxDepth(int depth) {
    m_maxDepth = depth;
    m_mcts->setMaxDepth(depth);
}
Playstyle MCTSDecisionMaker::getPlaystyle() const {
    return m_playstyle;
}
size_t MCTSDecisionMaker::selectCard(const std::vector<size_t>& available) {
    if (available.empty()) return 0;
    if (available.size() == 1) return 0;
    std::cout << "[" << playstyleToString(m_playstyle) << " AI] Thinking...\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
    return dist(gen);
}
int MCTSDecisionMaker::selectCardAction() {
    if (m_playstyle == Playstyle::BRITNEY) {
        return 0; 
    } else {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 9);
        return (dist(gen) < 7) ? 0 : 2;
    }
}
size_t MCTSDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
    if (candidates.empty()) return 0;
    if (candidates.size() == 1) return 0;
    return 0;
}
std::uint8_t MCTSDecisionMaker::selectStartingPlayer() {
    return (m_playstyle == Playstyle::SPEARS) ? 0 : 1;
}
HumanAssistedDecisionMaker::HumanAssistedDecisionMaker(Playstyle suggestionStyle, int iterations)
    : m_suggestionStyle(suggestionStyle)
    , m_iterations(iterations)
{
    m_mcts = std::make_unique<MCTS>(iterations, 1.414, 20, suggestionStyle);
    std::cout << "[Assistant] AI suggestions using " << playstyleToString(suggestionStyle) << " strategy\n";
}
HumanAssistedDecisionMaker::~HumanAssistedDecisionMaker() = default;
void HumanAssistedDecisionMaker::setSuggestionStyle(Playstyle style) {
    m_suggestionStyle = style;
    m_mcts->setPlaystyle(style);
    std::cout << "[Assistant] Switched to " << playstyleToString(style) << " suggestions\n";
}
Playstyle HumanAssistedDecisionMaker::getSuggestionStyle() const {
    return m_suggestionStyle;
}
void HumanAssistedDecisionMaker::showSuggestions(const std::vector<size_t>& options, const std::string& context) {
    if (options.empty()) return;
    
    std::cout << "\n[" << playstyleToString(m_suggestionStyle) << "]: AI Suggestions for " << context << ":\n";
    
    // Evaluate each option using simple heuristics
    std::vector<std::pair<size_t, double>> rankedOptions; // {position_in_available, score}
    
    auto& board = Board::getInstance();
    
    for (size_t pos = 0; pos < options.size(); ++pos) {
        size_t nodeIdx = options[pos];  // Internal node index (14, 15, etc)
        
        // Get the actual card at this node - check all possible age phases
        Models::Card* card = nullptr;
        const auto& nodes1 = board.getAge1Nodes();
        const auto& nodes2 = board.getAge2Nodes();
        const auto& nodes3 = board.getAge3Nodes();
        
        if (nodeIdx < nodes1.size() && nodes1[nodeIdx] && nodes1[nodeIdx]->getCard()) {
            card = const_cast<Models::Card*>(nodes1[nodeIdx]->getCard());
        } else if (nodeIdx < nodes2.size() && nodes2[nodeIdx] && nodes2[nodeIdx]->getCard()) {
            card = const_cast<Models::Card*>(nodes2[nodeIdx]->getCard());
        } else if (nodeIdx < nodes3.size() && nodes3[nodeIdx] && nodes3[nodeIdx]->getCard()) {
            card = const_cast<Models::Card*>(nodes3[nodeIdx]->getCard());
        }
        
        double score = 0.0;
        
        if (card) {
            // Evaluate card based on playstyle
            if (m_suggestionStyle == Playstyle::BRITNEY) {
                // Britney: Victory Points > Military
                score += card->getVictoryPoints() * 2.0;
                if (auto* ageCard = dynamic_cast<Models::AgeCard*>(card)) {
                    score += ageCard->getShieldPoints() * 0.5;
                    if (ageCard->getScientificSymbols().has_value()) {
                        score += 3.0;
                    }
                }
            } else {
                // Spears: Military > Victory Points
                if (auto* ageCard = dynamic_cast<Models::AgeCard*>(card)) {
                    score += ageCard->getShieldPoints() * 3.0;
                    score += card->getVictoryPoints() * 0.5;
                }
            }
            
            // Add small randomness
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> noise(0.0, 0.1);
            score += noise(gen);
        } else {
            score = 0.1; // Fallback for missing cards
        }
        
        rankedOptions.push_back({pos, score});  // Store position (0-5), not nodeIdx
    }
    
    // Sort by score (descending)
    std::sort(rankedOptions.begin(), rankedOptions.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Display ranked suggestions
    size_t topPicks = std::min(size_t(3), rankedOptions.size());
    for (size_t i = 0; i < topPicks; ++i) {
        std::cout << "  [" << playstyleToString(m_suggestionStyle) << "]: ";
        if (i == 0) std::cout << "BEST CHOICE -> ";
        else if (i == 1) std::cout << "Good alternative -> ";
        else std::cout << "Consider -> ";
        std::cout << "Option " << rankedOptions[i].first 
                  << " (score: " << std::fixed << std::setprecision(2) << rankedOptions[i].second << ")\n";
    }
    std::cout << "\n";
}
size_t HumanAssistedDecisionMaker::selectCard(const std::vector<size_t>& available) {
    showSuggestions(available, "card selection");
    std::cout << "Your choice (0-" << (available.size() - 1) << "): ";
    size_t choice = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter a valid number (0-" << (available.size() - 1) << "): ";
            continue;
        }
        if (choice >= available.size()) {
            std::cout << "ERROR: Invalid choice! Please enter a number between 0 and " << (available.size() - 1) << ": ";
            continue;
        }
        validInput = true;
    }
    return choice;
}
int HumanAssistedDecisionMaker::selectCardAction() {
    std::vector<size_t> actions = {0, 1, 2}; 
    std::cout << "\n[" << playstyleToString(m_suggestionStyle) << "]: AI Suggestion for action:\n";
    if (m_suggestionStyle == Playstyle::BRITNEY) {
        std::cout << "  [Britney]: BUILD this card (accumulate victory points)\n";
    } else {
        std::cout << "  [Spears]: BUILD this card (strengthen military position)\n";
    }
    std::cout << "\nYour choice [0]=build, [1]=sell, [2]=wonder: ";
    int action = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> action;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter 0, 1, or 2: ";
            continue;
        }
        if (action < 0 || action > 2) {
            std::cout << "ERROR: Invalid action! Please enter 0 (build), 1 (sell), or 2 (wonder): ";
            continue;
        }
        validInput = true;
    }
    return action;
}
size_t HumanAssistedDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
    showSuggestions(candidates, "wonder selection");
    std::cout << "Your choice (0-" << (candidates.size() - 1) << "): ";
    size_t choice = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter a valid number (0-" << (candidates.size() - 1) << "): ";
            continue;
        }
        if (choice >= candidates.size()) {
            std::cout << "ERROR: Invalid wonder choice! Please enter a number between 0 and " << (candidates.size() - 1) << ": ";
            continue;
        }
        validInput = true;
    }
    return choice;
}
std::uint8_t HumanAssistedDecisionMaker::selectStartingPlayer() {
    std::cout << "\n[" << playstyleToString(m_suggestionStyle) << "]: AI Suggestion:\n";
    if (m_suggestionStyle == Playstyle::SPEARS) {
        std::cout << "  [Spears]: GO FIRST (aggressive opening)\n";
    } else {
        std::cout << "  [Britney]: GO SECOND (gather information first)\n";
    }
    std::cout << "\nYour choice [0]=Player1, [1]=Player2: ";
    int choice = 0;
    std::cin >> choice;
    return (choice == 0) ? 0 : 1;
}
AIDecisionMaker::AIDecisionMaker() {
    m_mcts = std::make_unique<MCTS>(m_simulationCount, m_explorationConstant, m_maxDepth);
}
AIDecisionMaker::~AIDecisionMaker() = default;
void AIDecisionMaker::setSimulationCount(unsigned int count) {
    m_simulationCount = count;
    m_mcts->setIterations(static_cast<int>(count));
}
void AIDecisionMaker::setExplorationConstant(double constant) {
    m_explorationConstant = constant;
    m_mcts->setExplorationConstant(constant);
}
void AIDecisionMaker::setMaxDepth(unsigned int depth) {
    m_maxDepth = depth;
    m_mcts->setMaxDepth(static_cast<int>(depth));
}
size_t AIDecisionMaker::selectCard(const std::vector<size_t>& available) {
    if (available.empty()) return 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
    return dist(gen);
}
int AIDecisionMaker::selectCardAction() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 2);
    return dist(gen);
}
size_t AIDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
    if (candidates.empty()) return 0;
    return 0;
}
std::uint8_t AIDecisionMaker::selectStartingPlayer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1);
    return static_cast<std::uint8_t>(dist(gen));
}
} 
