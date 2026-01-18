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
import Models.Token;
import Core.IGameListener;
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
size_t HumanDecisionMaker::selectProgressToken(const std::vector<size_t>& available) {
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
            std::cout << "ERROR: Invalid token choice! Please enter a number between 0 and " << (available.size() - 1) << ": ";
            continue;
        }
        validInput = true;
    }
    return choice;
}
size_t HumanDecisionMaker::selectCardToDiscard(const std::vector<size_t>& availableCards) {
    size_t choice = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter a valid number (0-" << (availableCards.size() - 1) << "): ";
            continue;
        }
        if (choice >= availableCards.size()) {
            std::cout << "ERROR: Invalid card choice! Please enter a number between 0 and " << (availableCards.size() - 1) << ": ";
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
MCTSAction MCTSDecisionMaker::selectTurnAction() {
    auto& gs = GameState::getInstance();
    const int phase = gs.getCurrentPhase();
    const bool isP1Turn = gs.isPlayer1Turn();

    // Keep the underlying MCTS instance in sync with any runtime tuning.
    if (m_mcts) {
        m_mcts->setIterations(m_iterations);
        m_mcts->setExplorationConstant(m_explorationConstant);
        m_mcts->setMaxDepth(m_maxDepth);
        m_mcts->setPlaystyle(m_playstyle);
    }

    MCTSGameState state = MCTS::captureGameState(phase, isP1Turn);
    if (!m_mcts) return MCTSAction{};
    return m_mcts->search(state);
}
size_t MCTSDecisionMaker::selectCard(const std::vector<size_t>& available) {
    if (available.empty()) return 0;
    if (available.size() == 1) return 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
    return dist(gen);
}
int MCTSDecisionMaker::selectCardAction() {
    auto currentPlayer = getCurrentPlayer();
    if (!currentPlayer || !currentPlayer->m_player) {
        return (m_playstyle == Playstyle::BRITNEY) ? 0 : 0;
    }
    
    auto& board = Board::getInstance();
    uint8_t coins = currentPlayer->m_player->totalCoins(currentPlayer->m_player->getRemainingCoins());
    
    auto& ownedWonders = currentPlayer->m_player->getOwnedWonders();
    int unbuildWonders = 0;
    for (const auto& w : ownedWonders) {
        if (w && !w->IsConstructed()) unbuildWonders++;
    }
    
    if (m_playstyle == Playstyle::BRITNEY) {
        if (coins < 3) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, 99);
            return (dist(gen) < 70) ? 1 : 0;
        }
        
        if (unbuildWonders > 0 && coins >= 4) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, 99);
            if (dist(gen) < 15) return 2;
        }
        
        return 0;
        
    } else {
        if (coins < 2) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, 99);
            return (dist(gen) < 60) ? 1 : 0;
        }
        
        if (unbuildWonders > 0 && coins >= 3) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, 99);
            if (dist(gen) < 35) return 2;
        }
        
        if (coins < 5) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, 99);
            if (dist(gen) < 20) return 1;
        }
        
        return 0;
    }
}
size_t MCTSDecisionMaker::selectWonder(const std::vector<size_t>& candidates) {
    if (candidates.empty()) return 0;
    if (candidates.size() == 1) return 0;

    auto currentPlayer = getCurrentPlayer();
    if (!currentPlayer || !currentPlayer->m_player) return 0;

    AIConfig config(m_playstyle);
    const auto weights = config.getWeights();

    const auto& owned = currentPlayer->m_player->getOwnedWonders();

    auto scoreWonder = [&](const Models::Wonder* w) -> double {
        if (!w) return -1e9;
        double score = 0.0;

        score += static_cast<double>(w->getVictoryPoints()) * weights.victoryPointValue * weights.wonderVPBonus;
        score += static_cast<double>(w->getShieldPoints()) * weights.militaryPriority * weights.wonderMilitaryBonus;

        if (w->getResourceProduction() != Models::ResourceType::NO_RESOURCE) {
            score += 1.0 * weights.resourceValue;
        }

        // Slightly penalize expensive wonders so the AI prefers buildable ones.
        for (const auto& [res, amt] : w->getResourceCost()) {
            (void)res;
            score -= static_cast<double>(amt) * weights.resourceValue * 0.5;
        }

        return score;
    };

    size_t bestPos = 0;
    double bestScore = -1e18;
    for (size_t pos = 0; pos < candidates.size(); ++pos) {
        size_t ownedIdx = candidates[pos];
        if (ownedIdx >= owned.size() || !owned[ownedIdx]) continue;
        const auto* w = owned[ownedIdx].get();
        if (w->IsConstructed()) continue;

        double s = scoreWonder(w);
        if (s > bestScore) {
            bestScore = s;
            bestPos = pos;
        }
    }

    return bestPos;
}
size_t MCTSDecisionMaker::selectProgressToken(const std::vector<size_t>& available) {
    if (available.empty()) return 0;
    if (available.size() == 1) return 0;
    
    auto& board = Board::getInstance();
    const auto& tokens = board.getProgressTokens();

    AIConfig config(m_playstyle);
    const auto weights = config.getWeights();

    auto totalCoinValue = [](const std::tuple<uint8_t, uint8_t, uint8_t>& coins) -> uint32_t {
        return static_cast<uint32_t>(std::get<0>(coins)) +
               static_cast<uint32_t>(std::get<1>(coins)) * 3u +
               static_cast<uint32_t>(std::get<2>(coins)) * 6u;
    };

    // Needed for Mathematics token evaluation.
    size_t currentTokenCount = 0;
    if (auto cp = getCurrentPlayer(); cp && cp->m_player) {
        currentTokenCount = cp->m_player->getOwnedTokens().size();
    }

    auto scoreToken = [&](const Models::Token* token) -> double {
        if (!token) return -1e9;
        const std::string& name = token->getName();
        double score = 0.0;

        score += static_cast<double>(token->getVictoryPoints()) * weights.victoryPointValue;
        score += static_cast<double>(totalCoinValue(token->getCoins())) * weights.coinValue;
        score += static_cast<double>(token->getShieldPoints()) * weights.militaryPriority;

        // Weight-driven approximations for "future value" progress tokens.
        if (name == "Architecture") {
            score += 2.0 * weights.wonderEconomyBonus + 1.0 * weights.resourceValue;
        }
        else if (name == "Masonry") {
            score += 2.0 * weights.economyPriority + 1.0 * weights.victoryPointValue;
        }
        else if (name == "Economy") {
            score += 2.0 * weights.economyPriority + 0.5 * weights.opponentDenial;
        }
        else if (name == "Law") {
            score += 3.0 * weights.sciencePriority;
        }
        else if (name == "Mathematics") {
            // 3 VP per progress token (including itself) at end of game.
            score += static_cast<double>(3u * (currentTokenCount + 1u)) * weights.victoryPointValue;
            score += 1.0 * weights.sciencePriority;
        }
        else if (name == "Strategy") {
            score += 2.0 * weights.militaryPriority;
        }
        else if (name == "Theology") {
            score += 2.0 * weights.wonderEconomyBonus;
        }
        else if (name == "Urbanism") {
            score += 1.0 * weights.economyPriority;
        }
        else if (name == "Agriculture") {
            score += 1.0 * weights.economyPriority;
        }
        else if (name == "Philosophy") {
            // Already covered via VP above; keep as-is.
        }

        return score;
    };

    size_t bestPos = 0;
    double bestScore = -1e18;

    for (size_t pos = 0; pos < available.size(); ++pos) {
        size_t tokenIdx = available[pos];
        if (tokenIdx >= tokens.size() || !tokens[tokenIdx]) continue;
        double s = scoreToken(tokens[tokenIdx].get());
        if (s > bestScore) {
            bestScore = s;
            bestPos = pos;
        }
    }

    return bestPos;
}
size_t MCTSDecisionMaker::selectCardToDiscard(const std::vector<size_t>& availableCards) {
    if (availableCards.empty()) return 0;
    if (availableCards.size() == 1) return 0;
    
    auto currentPlayer = getCurrentPlayer();
    if (!currentPlayer || !currentPlayer->m_player) return 0;
    
    auto opponent = getOpponentPlayer();
    if (!opponent || !opponent->m_player) return 0;
    
    const auto& ownedCards = opponent->m_player->getOwnedCards();

    AIConfig config(m_playstyle);
    const auto weights = config.getWeights();

    auto scoreOpponentCard = [&](const Models::Card* card) -> double {
        if (!card) return -1e9;
        double score = 0.0;

        // Removing opponent points/resources/science/military is "denial".
        score += static_cast<double>(card->getVictoryPoints()) * weights.victoryPointValue * weights.opponentDenial;

        if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
            score += static_cast<double>(ageCard->getShieldPoints()) * weights.militaryPriority * weights.opponentDenial;
            if (ageCard->getScientificSymbols().has_value()) {
                score += 2.0 * weights.sciencePriority * weights.opponentDenial;
            }
            for (const auto& [res, amt] : ageCard->getResourcesProduction()) {
                (void)res;
                score += static_cast<double>(amt) * weights.resourceValue * weights.opponentDenial;
            }
        }

        // Deny economy cards a bit (yellow typically grants coins/discounts).
        if (card->getColor() == Models::ColorType::YELLOW) {
            score += 1.0 * weights.economyPriority * weights.opponentDenial;
        }

        return score;
    };

    size_t bestPos = 0;
    double bestScore = -1e18;

    for (size_t pos = 0; pos < availableCards.size(); ++pos) {
        size_t cardIdx = availableCards[pos];
        if (cardIdx >= ownedCards.size() || !ownedCards[cardIdx]) continue;
        double s = scoreOpponentCard(ownedCards[cardIdx].get());
        if (s > bestScore) {
            bestScore = s;
            bestPos = pos;
        }
    }

    return bestPos;
}
std::uint8_t MCTSDecisionMaker::selectStartingPlayer() {
    AIConfig config(m_playstyle);
    const auto weights = config.getWeights();

    // Simple trainable heuristic: higher military/denial -> prefer going first.
    double aggression = weights.militaryPriority + weights.opponentDenial;
    double economy = weights.victoryPointValue + weights.sciencePriority + weights.economyPriority;
    return (aggression >= economy) ? 0 : 1;
}
HumanAssistedDecisionMaker::HumanAssistedDecisionMaker(Playstyle suggestionStyle, int iterations)
    : m_suggestionStyle(suggestionStyle)
    , m_iterations(iterations)
{
    m_mcts = std::make_unique<MCTS>(iterations, 1.414, 20, suggestionStyle);
}
HumanAssistedDecisionMaker::~HumanAssistedDecisionMaker() = default;
void HumanAssistedDecisionMaker::setSuggestionStyle(Playstyle style) {
    m_suggestionStyle = style;
    m_mcts->setPlaystyle(style);
}
Playstyle HumanAssistedDecisionMaker::getSuggestionStyle() const {
    return m_suggestionStyle;
}
void HumanAssistedDecisionMaker::showSuggestions(const std::vector<size_t>& options, const std::string& context) {
    if (options.empty()) return;
    
    auto& notifier = GameState::getInstance().getEventNotifier();
    DisplayRequestEvent event;
    event.displayType = DisplayRequestEvent::Type::MESSAGE;
    event.context = "\n[" + playstyleToString(m_suggestionStyle) + "]: AI Suggestions for " + context + ":";
    notifier.notifyDisplayRequested(event);
    
    std::vector<std::pair<size_t, double>> rankedOptions;
    
    auto& board = Board::getInstance();
    
    for (size_t pos = 0; pos < options.size(); ++pos) {
        size_t nodeIdx = options[pos];
        
        std::optional<std::reference_wrapper<const Models::Card>> card = std::nullopt;
        const auto& nodes1 = board.getAge1Nodes();
        const auto& nodes2 = board.getAge2Nodes();
        const auto& nodes3 = board.getAge3Nodes();
        
        if (nodeIdx < nodes1.size() && nodes1[nodeIdx]) {
            card = nodes1[nodeIdx]->getCard();
        } else if (nodeIdx < nodes2.size() && nodes2[nodeIdx]) {
            card = nodes2[nodeIdx]->getCard();
        } else if (nodeIdx < nodes3.size() && nodes3[nodeIdx]) {
            card = nodes3[nodeIdx]->getCard();
        }
        
        double score = 0.0;
        
        if (card.has_value()) {
            if (m_suggestionStyle == Playstyle::BRITNEY) {
                score += card->get().getVictoryPoints() * 2.0;
                if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(&card->get())) {
                    score += ageCard->getShieldPoints() * 0.5;
                    if (ageCard->getScientificSymbols().has_value()) {
                        score += 3.0;
                    }
                }
            } else {
                if (auto* ageCard = dynamic_cast<const Models::AgeCard*>(&card->get())) {
                    score += ageCard->getShieldPoints() * 3.0;
                    score += card->get().getVictoryPoints() * 0.5;
                }
            }
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> noise(0.0, 0.1);
            score += noise(gen);
        } else {
            score = 0.1;
        }
        
        rankedOptions.push_back({pos, score});
    }
    
    std::sort(rankedOptions.begin(), rankedOptions.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    size_t topPicks = std::min(size_t(3), rankedOptions.size());
    for (size_t i = 0; i < topPicks; ++i) {
        event.context = "  [" + playstyleToString(m_suggestionStyle) + "]: ";
        if (i == 0) event.context += "BEST CHOICE -> ";
        else if (i == 1) event.context += "Good alternative -> ";
        else event.context += "Consider -> ";
        event.context += "Option " + std::to_string(rankedOptions[i].first) 
                      + " (score: " + std::to_string(rankedOptions[i].second) + ")";
        notifier.notifyDisplayRequested(event);
    }
    
    event.context = "";
    notifier.notifyDisplayRequested(event);
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
size_t HumanAssistedDecisionMaker::selectProgressToken(const std::vector<size_t>& available) {
    showSuggestions(available, "progress token selection");
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
            std::cout << "ERROR: Invalid token choice! Please enter a number between 0 and " << (available.size() - 1) << ": ";
            continue;
        }
        validInput = true;
    }
    return choice;
}
size_t HumanAssistedDecisionMaker::selectCardToDiscard(const std::vector<size_t>& availableCards) {
    showSuggestions(availableCards, "card discard selection");
    std::cout << "Your choice (0-" << (availableCards.size() - 1) << "): ";
    size_t choice = 0;
    bool validInput = false;
    while (!validInput) {
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "ERROR: Invalid input! Please enter a valid number (0-" << (availableCards.size() - 1) << "): ";
            continue;
        }
        if (choice >= availableCards.size()) {
            std::cout << "ERROR: Invalid card choice! Please enter a number between 0 and " << (availableCards.size() - 1) << ": ";
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
MCTSAction AIDecisionMaker::selectTurnAction() {
    auto& gs = GameState::getInstance();
    const int phase = gs.getCurrentPhase();
    const bool isP1Turn = gs.isPlayer1Turn();

    if (m_mcts) {
        m_mcts->setIterations(static_cast<int>(m_simulationCount));
        m_mcts->setExplorationConstant(m_explorationConstant);
        m_mcts->setMaxDepth(static_cast<int>(m_maxDepth));
    }

    MCTSGameState state = MCTS::captureGameState(phase, isP1Turn);
    if (!m_mcts) return MCTSAction{};
    return m_mcts->search(state);
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
size_t AIDecisionMaker::selectProgressToken(const std::vector<size_t>& available) {
    if (available.empty()) return 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
    return dist(gen);
}
size_t AIDecisionMaker::selectCardToDiscard(const std::vector<size_t>& availableCards) {
    if (availableCards.empty()) return 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, availableCards.size() - 1);
    return dist(gen);
}
std::uint8_t AIDecisionMaker::selectStartingPlayer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1);
    return static_cast<std::uint8_t>(dist(gen));
}
}
