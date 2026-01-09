module Core.PlayerDecisionMaker;
import <vector>;
import <iostream>;
import <memory>;
import <random>;
import <algorithm>;
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
    std::cin >> choice;
    if (choice >= available.size()) {
        std::cout << "Invalid choice, defaulting to 0\n";
        choice = 0;
    }
    return choice;
}
int HumanDecisionMaker::selectCardAction() {
    int action = 0;
    std::cin >> action;
    if (action < 0 || action > 2) {
        std::cout << "Invalid action, defaulting to 0 (build)\n";
        action = 0;
    }
    return action;
}
size_t HumanDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
    size_t choice = 0;
    std::cin >> choice;
    if (choice >= candidates.size()) {
        std::cout << "Invalid wonder choice, defaulting to 0\n";
        choice = 0;
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
    size_t topPicks = std::min(size_t(3), options.size());
    for (size_t i = 0; i < topPicks; ++i) {
        std::cout << "  [" << playstyleToString(m_suggestionStyle) << "]: ";
        if (i == 0) std::cout << "BEST CHOICE -> ";
        else if (i == 1) std::cout << "Good alternative -> ";
        else std::cout << "Consider -> ";
        std::cout << "Option " << i << "\n";
    }
    std::cout << "\n";
}
size_t HumanAssistedDecisionMaker::selectCard(const std::vector<size_t>& available) {
    showSuggestions(available, "card selection");
    std::cout << "Your choice (0-" << (available.size() - 1) << "): ";
    size_t choice = 0;
    std::cin >> choice;
    if (choice >= available.size()) {
        std::cout << "Invalid choice, defaulting to 0\n";
        choice = 0;
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
    std::cin >> action;
    if (action < 0 || action > 2) {
        std::cout << "Invalid action, defaulting to 0 (build)\n";
        action = 0;
    }
    return action;
}
size_t HumanAssistedDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
    showSuggestions(candidates, "wonder selection");
    std::cout << "Your choice (0-" << (candidates.size() - 1) << "): ";
    size_t choice = 0;
    std::cin >> choice;
    if (choice >= candidates.size()) {
        std::cout << "Invalid wonder choice, defaulting to 0\n";
        choice = 0;
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
