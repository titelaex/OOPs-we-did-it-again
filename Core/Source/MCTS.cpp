module Core.MCTS;
import <vector>;
import <memory>;
import <random>;
import <algorithm>;
import <iostream>;
import <sstream>;
import <fstream>;
#include <cmath>
#include <limits>
import Core.Player;
import Core.Board;
import Core.GameState;
import Core.Node;
import Core.AIConfig;
import Models.AgeCard;
import Models.Wonder;
import Models.Card;
import Models.ColorType;
namespace Core {
namespace {
    CardInfo extractCardInfo(const Models::Card* card) {
        CardInfo info;
        if (!card) return info;
        info.name = card->getName();
        info.color = card->getColor();
        info.victoryPoints = card->getVictoryPoints();
        info.resourceCost = card->getResourceCost();
        if (const auto* ageCard = dynamic_cast<const Models::AgeCard*>(card)) {
            info.shieldPoints = ageCard->getShieldPoints();
            info.resourcesProduction = ageCard->getResourcesProduction();
            if (ageCard->getScientificSymbols().has_value()) {
                info.hasScientificSymbol = true;
                info.scientificSymbol = ageCard->getScientificSymbols().value();
            }
        }
        if (const auto* wonder = dynamic_cast<const Models::Wonder*>(card)) {
            info.shieldPoints = wonder->getShieldPoints();
            info.isConstructed = wonder->IsConstructed();
            auto resProd = wonder->getResourceProduction();
            if (resProd != Models::ResourceType::NO_RESOURCE) {
                info.resourcesProduction[resProd] = 1;
            }
        }
        return info;
    }
    PlayerInfo extractPlayerInfo(const std::shared_ptr<Player>& player) {
        PlayerInfo info;
        if (!player || !player->m_player) return info;
        auto& modelPlayer = player->m_player;
        auto coins = modelPlayer->getRemainingCoins();
        info.coins = modelPlayer->totalCoins(coins);
        info.cardCount = static_cast<uint8_t>(modelPlayer->getOwnedCards().size());
        info.wonderCount = static_cast<uint8_t>(modelPlayer->getOwnedWonders().size());
        info.tokenCount = static_cast<uint8_t>(modelPlayer->getOwnedTokens().size());
        for (const auto& card : modelPlayer->getOwnedCards()) {
            if (card) {
                CardInfo cardInfo = extractCardInfo(card.get());
                info.ownedCards.push_back(cardInfo);
                switch (cardInfo.color) {
                    case Models::ColorType::BROWN: info.brownCards++; break;
                    case Models::ColorType::GREY: info.greyCards++; break;
                    case Models::ColorType::BLUE: info.blueCards++; break;
                    case Models::ColorType::RED: info.redCards++; break;
                    case Models::ColorType::GREEN: info.greenCards++; break;
                    case Models::ColorType::YELLOW: info.yellowCards++; break;
                    case Models::ColorType::PURPLE: info.purpleCards++; break;
                    default: break;
                }
            }
        }
        for (const auto& wonder : modelPlayer->getOwnedWonders()) {
            if (wonder) {
                CardInfo wonderInfo = extractCardInfo(wonder.get());
                info.ownedWonders.push_back(wonderInfo);
            }
        }
        info.permanentResources = modelPlayer->getOwnedPermanentResources();
        info.tradingResources = modelPlayer->getOwnedTradingResources();
        info.scientificSymbols = modelPlayer->getOwnedScientificSymbols();
        const auto& points = modelPlayer->getPoints();
        info.militaryVP = points.m_militaryVictoryPoints;
        info.buildingVP = points.m_buildingVictoryPoints;
        info.wonderVP = points.m_wonderVictoryPoints;
        info.progressVP = points.m_progressVictoryPoints;
        return info;
    }
}
MCTSNode::MCTSNode(const MCTSGameState& state, MCTSNode* parent, const MCTSAction& action)
    : m_state(state)
    , m_action(action)
    , m_parent(parent)
    , m_visits(0)
    , m_totalValue(0.0)
    , m_untriedActionsIndex(0)
{
}
double MCTSNode::getUCB1Score(double explorationConstant) const {
    if (m_visits == 0) {
        return std::numeric_limits<double>::infinity();
    }
    double exploitation = m_totalValue / static_cast<double>(m_visits);
    double exploration = 0.0;
    if (m_parent && m_parent->m_visits > 0) {
        exploration = explorationConstant * std::sqrt(
            std::log(static_cast<double>(m_parent->m_visits)) / 
            static_cast<double>(m_visits)
        );
    }
    return exploitation + exploration;
}
bool MCTSNode::isFullyExpanded() const {
    return m_untriedActionsIndex >= m_state.availableActions.size();
}
MCTSNode* MCTSNode::select(double explorationConstant) {
    MCTSNode* current = this;
    while (!current->m_state.isTerminal() && current->isFullyExpanded() && !current->isLeaf()) {
        MCTSNode* best = nullptr;
        double bestScore = -std::numeric_limits<double>::infinity();
        for (const auto& child : current->m_children) {
            double score = child->getUCB1Score(explorationConstant);
            if (score > bestScore) {
                bestScore = score;
                best = child.get();
            }
        }
        if (best) {
            current = best;
        } else {
            break;
        }
    }
    return current;
}
MCTSNode* MCTSNode::expand() {
    if (m_state.isTerminal() || m_untriedActionsIndex >= m_state.availableActions.size()) {
        return this;
    }
    const MCTSAction& action = m_state.availableActions[m_untriedActionsIndex++];
    MCTSGameState newState = MCTS::cloneState(m_state);
    MCTS::applyAction(newState, action);
    auto child = std::make_unique<MCTSNode>(newState, this, action);
    MCTSNode* childPtr = child.get();
    m_children.push_back(std::move(child));
    return childPtr;
}
double MCTSNode::simulate(std::mt19937& rng, int maxDepth) {
    MCTSGameState simState = MCTS::cloneState(m_state);
    int depth = 0;
    while (!MCTS::isTerminal(simState) && depth < maxDepth) {
        std::vector<MCTSAction> possibleActions = simState.availableActions;
        if (possibleActions.empty()) {
            possibleActions = MCTS::getLegalActions(simState, simState.currentPhase);
        }
        if (possibleActions.empty()) break;
        std::uniform_int_distribution<size_t> dist(0, possibleActions.size() - 1);
        const MCTSAction& randomAction = possibleActions[dist(rng)];
        MCTS::applyAction(simState, randomAction);
        depth++;
    }
    double score = 0.0;
    AIConfig config(Playstyle::BRITNEY);
    auto weights = config.getWeights();
    if (MCTS::isTerminal(simState)) {
        int winner = MCTS::getWinner(simState);
        if (m_state.player1Turn) {
            if (winner == 0) score = 1.0;
            else if (winner == 1) score = 0.0;
            else score = 0.5;
        } else {
            if (winner == 1) score = 1.0;
            else if (winner == 0) score = 0.0;
            else score = 0.5;
        }
    } else {
        double p1Score = simState.player1.coins * weights.coinValue +
                         simState.player1.cardCount * weights.economyPriority +
                         simState.player1.blueCards * weights.victoryPointValue +
                         simState.player1.greenCards * weights.sciencePriority +
                         simState.player1.redCards * weights.militaryPriority;
        double p2Score = simState.player2.coins * weights.coinValue +
                         simState.player2.cardCount * weights.economyPriority +
                         simState.player2.blueCards * weights.victoryPointValue +
                         simState.player2.greenCards * weights.sciencePriority +
                         simState.player2.redCards * weights.militaryPriority;
        if (simState.militaryPosition < 9) {
            p1Score += (9 - simState.militaryPosition) * weights.militaryPriority;
        } else {
            p2Score += (simState.militaryPosition - 9) * weights.militaryPriority;
        }
        size_t p1ScienceTypes = simState.player1.scientificSymbols.size();
        size_t p2ScienceTypes = simState.player2.scientificSymbols.size();
        p1Score += p1ScienceTypes * weights.sciencePriority;
        p2Score += p2ScienceTypes * weights.sciencePriority;
        if (m_state.player1Turn) {
            score = (p1Score > p2Score) ? 1.0 : (p1Score == p2Score ? 0.5 : 0.0);
        } else {
            score = (p2Score > p1Score) ? 1.0 : (p1Score == p2Score ? 0.5 : 0.0);
        }
    }
    return score;
}
void MCTSNode::backpropagate(double reward) {
    MCTSNode* current = this;
    while (current != nullptr) {
        current->m_visits++;
        current->m_totalValue += reward;
        reward = 1.0 - reward;
        current = current->m_parent;
    }
}
MCTS::MCTS(int iterations, double explorationConstant, int maxSimulationDepth, Playstyle playstyle)
    : m_iterations(iterations)
    , m_explorationConstant(explorationConstant)
    , m_maxSimulationDepth(maxSimulationDepth)
    , m_rng(std::random_device{}())
    , m_playstyle(playstyle)
{
}
MCTSAction MCTS::search(const MCTSGameState& rootState) {
    MCTSGameState stateWithActions = cloneState(rootState);
    if (stateWithActions.availableActions.empty()) {
        stateWithActions.availableActions = getLegalActions(stateWithActions, stateWithActions.currentPhase);
    }
    if (stateWithActions.availableActions.empty()) {
        return MCTSAction{};
    }
    auto root = std::make_unique<MCTSNode>(stateWithActions);
    for (int i = 0; i < m_iterations; ++i) {
        MCTSNode* node = root->select(m_explorationConstant);
        if (!node->getState().isTerminal() && !node->isFullyExpanded()) {
            node = node->expand();
        }
        double reward = node->simulate(m_rng, m_maxSimulationDepth);
        node->backpropagate(reward);
    }
    MCTSNode* bestChild = nullptr;
    int maxVisits = -1;
    for (const auto& child : root->getChildren()) {
        if (child && child->getVisits() > maxVisits) {
            maxVisits = child->getVisits();
            bestChild = child.get();
        }
    }
    if (bestChild) {
        std::cout << "[MCTS] Best action found with " << maxVisits << " visits out of " << m_iterations << " iterations\n";
        return bestChild->getAction();
    }
    std::cout << "[MCTS] No best child found, using first action\n";
    return stateWithActions.availableActions[0];
}
MCTSGameState MCTS::captureGameState(int currentPhase, bool isPlayer1Turn) {
    MCTSGameState state;
    auto& gameState = Core::GameState::getInstance();
    auto& board = Core::Board::getInstance();
    state.currentPhase = currentPhase;
    state.player1Turn = isPlayer1Turn;
    state.militaryPosition = board.getPawnPos();
    state.roundsRemaining = 20; 
    state.gameOver = false;
    state.winner = -1;
    if (auto* p1 = gameState.GetPlayer1()->m_player.get()) {
        state.player1.coins = p1->totalCoins(p1->getRemainingCoins());
        state.player1.cardCount = static_cast<uint8_t>(p1->getOwnedCards().size());
        state.player1.wonderCount = static_cast<uint8_t>(p1->getOwnedWonders().size());
        state.player1.tokenCount = static_cast<uint8_t>(p1->getOwnedTokens().size());
        for (const auto& card : p1->getOwnedCards()) {
            if (!card) continue;
            switch (card->getColor()) {
                case Models::ColorType::BLUE: state.player1.blueCards++; break;
                case Models::ColorType::GREEN: state.player1.greenCards++; break;
                case Models::ColorType::RED: state.player1.redCards++; break;
                case Models::ColorType::YELLOW: state.player1.yellowCards++; break;
                case Models::ColorType::PURPLE: state.player1.purpleCards++; break;
                case Models::ColorType::BROWN: state.player1.brownCards++; break;
                case Models::ColorType::GREY: state.player1.greyCards++; break;
                default: break;
            }
        }
        state.player1.permanentResources = p1->getOwnedPermanentResources();
        state.player1.tradingResources = p1->getOwnedTradingResources();
        state.player1.scientificSymbols = p1->getOwnedScientificSymbols();
        const auto& pts1 = p1->getPoints();
        state.player1.militaryVP = pts1.m_militaryVictoryPoints;
        state.player1.buildingVP = pts1.m_buildingVictoryPoints;
        state.player1.wonderVP = pts1.m_wonderVictoryPoints;
        state.player1.progressVP = pts1.m_progressVictoryPoints;
    }
    if (auto* p2 = gameState.GetPlayer2()->m_player.get()) {
        state.player2.coins = p2->totalCoins(p2->getRemainingCoins());
        state.player2.cardCount = static_cast<uint8_t>(p2->getOwnedCards().size());
        state.player2.wonderCount = static_cast<uint8_t>(p2->getOwnedWonders().size());
        state.player2.tokenCount = static_cast<uint8_t>(p2->getOwnedTokens().size());
        for (const auto& card : p2->getOwnedCards()) {
            if (!card) continue;
            switch (card->getColor()) {
                case Models::ColorType::BLUE: state.player2.blueCards++; break;
                case Models::ColorType::GREEN: state.player2.greenCards++; break;
                case Models::ColorType::RED: state.player2.redCards++; break;
                case Models::ColorType::YELLOW: state.player2.yellowCards++; break;
                case Models::ColorType::PURPLE: state.player2.purpleCards++; break;
                case Models::ColorType::BROWN: state.player2.brownCards++; break;
                case Models::ColorType::GREY: state.player2.greyCards++; break;
                default: break;
            }
        }
        state.player2.permanentResources = p2->getOwnedPermanentResources();
        state.player2.tradingResources = p2->getOwnedTradingResources();
        state.player2.scientificSymbols = p2->getOwnedScientificSymbols();
        const auto& pts2 = p2->getPoints();
        state.player2.militaryVP = pts2.m_militaryVictoryPoints;
        state.player2.buildingVP = pts2.m_buildingVictoryPoints;
        state.player2.wonderVP = pts2.m_wonderVictoryPoints;
        state.player2.progressVP = pts2.m_progressVictoryPoints;
    }
    return state;
}
void MCTS::saveCurrentState(MCTSGameState& mctsState) {
    auto& gameState = GameState::getInstance();
    gameState.saveGameState("temp_mcts_state.csv");
    std::ostringstream oss;
    auto& board = Board::getInstance();
    oss << board;
    auto player1 = gameState.GetPlayer1();
    auto player2 = gameState.GetPlayer2();
    if (player1) oss << *player1;
    if (player2) oss << *player2;
    mctsState.serializedState = oss.str();
}
void MCTS::restoreState(const MCTSGameState& mctsState) {
    if (mctsState.serializedState.empty()) return;
    std::istringstream iss(mctsState.serializedState);
    auto& gameState = GameState::getInstance();
    auto& board = Board::getInstance();
    iss >> board;
    auto player1 = gameState.GetPlayer1();
    auto player2 = gameState.GetPlayer2();
    if (player1) iss >> *player1;
    if (player2) iss >> *player2;
}
std::vector<MCTSAction> MCTS::getLegalActions(const MCTSGameState& state, int currentPhase) {
    std::vector<MCTSAction> actions;
    auto& board = Board::getInstance();
    const std::vector<std::shared_ptr<Node>>* nodes = nullptr;
    switch (currentPhase) {
        case 1: nodes = &board.getAge1Nodes(); break;
        case 2: nodes = &board.getAge2Nodes(); break;
        case 3: nodes = &board.getAge3Nodes(); break;
        default: return actions; 
    }
    if (!nodes) return actions;
    for (size_t i = 0; i < nodes->size(); ++i) {
        const auto& node = (*nodes)[i];
        if (!node) continue;
        if (!node->isAvailable()) continue;
        const auto* card = node->getCard();
        if (!card) continue;
        MCTSAction buildAction;
        buildAction.cardNodeIndex = i;
        buildAction.actionType = 0;
        buildAction.cardName = card->getName();
        buildAction.cardColor = card->getColor();
        buildAction.expectedVP = card->getVictoryPoints();
        actions.push_back(buildAction);
        MCTSAction sellAction;
        sellAction.cardNodeIndex = i;
        sellAction.actionType = 1;
        sellAction.cardName = card->getName();
        sellAction.cardColor = card->getColor();
        actions.push_back(sellAction);
        const PlayerInfo& currentPlayer = state.getCurrentPlayer();
        for (size_t w = 0; w < currentPlayer.ownedWonders.size(); ++w) {
            if (!currentPlayer.ownedWonders[w].isConstructed) {
                MCTSAction wonderAction;
                wonderAction.cardNodeIndex = i;
                wonderAction.actionType = 2;
                wonderAction.wonderIndex = w;
                wonderAction.cardName = card->getName();
                wonderAction.cardColor = card->getColor();
                wonderAction.expectedVP = currentPlayer.ownedWonders[w].victoryPoints;
                actions.push_back(wonderAction);
            }
        }
    }
    return actions;
}
void MCTS::applyAction(MCTSGameState& state, const MCTSAction& action) {
    PlayerInfo& currentPlayer = state.getCurrentPlayerMut();
    PlayerInfo& opponent = state.getOpponentMut();
    if (action.actionType == 0) { 
        CardInfo newCard;
        newCard.name = action.cardName;
        newCard.color = action.cardColor;
        newCard.victoryPoints = action.expectedVP;
        currentPlayer.ownedCards.push_back(newCard);
        currentPlayer.cardCount++;
        switch (action.cardColor) {
            case Models::ColorType::BROWN: currentPlayer.brownCards++; break;
            case Models::ColorType::GREY: currentPlayer.greyCards++; break;
            case Models::ColorType::BLUE: currentPlayer.blueCards++; break;
            case Models::ColorType::RED: currentPlayer.redCards++; break;
            case Models::ColorType::GREEN: currentPlayer.greenCards++; break;
            case Models::ColorType::YELLOW: currentPlayer.yellowCards++; break;
            case Models::ColorType::PURPLE: currentPlayer.purpleCards++; break;
            default: break;
        }
    } else if (action.actionType == 1) { 
        uint8_t coinsGained = static_cast<uint8_t>(2 + currentPlayer.yellowCards);
        currentPlayer.coins += coinsGained;
    } else if (action.actionType == 2) { 
        if (action.wonderIndex < currentPlayer.ownedWonders.size()) {
            currentPlayer.ownedWonders[action.wonderIndex].isConstructed = true;
            currentPlayer.wonderVP += currentPlayer.ownedWonders[action.wonderIndex].victoryPoints;
        }
    }
    state.player1Turn = !state.player1Turn;
    state.roundsRemaining--;
    if (state.roundsRemaining <= 0) {
        state.gameOver = true;
    }
    if (state.militaryPosition <= 0 || state.militaryPosition >= 18) {
        state.gameOver = true;
        state.winner = (state.militaryPosition <= 0) ? 0 : 1;
    }
    auto checkScience = [](const PlayerInfo& p) -> bool {
        size_t uniqueSymbols = 0;
        for (const auto& [symbol, count] : p.scientificSymbols) {
            if (count > 0) uniqueSymbols++;
        }
        return uniqueSymbols >= 6;
    };
    if (checkScience(state.player1) && checkScience(state.player2)) {
        state.gameOver = true;
        state.winner = 2; 
    } else if (checkScience(state.player1)) {
        state.gameOver = true;
        state.winner = 0;
    } else if (checkScience(state.player2)) {
        state.gameOver = true;
        state.winner = 1;
    }
}
bool MCTS::isTerminal(const MCTSGameState& state) {
    return state.isTerminal();
}
int MCTS::getWinner(const MCTSGameState& state) {
    if (!state.gameOver) return -1;
    if (state.winner >= 0) return state.winner;
    auto calculateScore = [](const PlayerInfo& p) -> uint32_t {
        return static_cast<uint32_t>(p.militaryVP) +
               static_cast<uint32_t>(p.buildingVP) +
               static_cast<uint32_t>(p.wonderVP) +
               static_cast<uint32_t>(p.progressVP) +
               static_cast<uint32_t>(p.coins / 3);
    };
    uint32_t score1 = calculateScore(state.player1);
    uint32_t score2 = calculateScore(state.player2);
    if (score1 > score2) return 0;
    if (score2 > score1) return 1;
    uint8_t blue1 = 0, blue2 = 0;
    for (const auto& card : state.player1.ownedCards) {
        if (card.color == Models::ColorType::BLUE) blue1 += card.victoryPoints;
    }
    for (const auto& card : state.player2.ownedCards) {
        if (card.color == Models::ColorType::BLUE) blue2 += card.victoryPoints;
    }
    if (blue1 > blue2) return 0;
    if (blue2 > blue1) return 1;
    return 2; 
}
MCTSGameState MCTS::cloneState(const MCTSGameState& state) {
    return state.clone();
}
double MCTS::evaluateState(const MCTSGameState& state, bool forPlayer1) const {
    const PlayerInfo& player = forPlayer1 ? state.player1 : state.player2;
    const PlayerInfo& opponent = forPlayer1 ? state.player2 : state.player1;
    AIConfig config(m_playstyle);
    auto weights = config.getWeights();
    double myScore = player.coins * weights.coinValue +
                     player.cardCount * weights.economyPriority +
                     player.blueCards * weights.victoryPointValue +
                     player.greenCards * weights.sciencePriority +
                     player.redCards * weights.militaryPriority +
                     player.yellowCards * weights.economyPriority +
                     player.buildingVP * weights.victoryPointValue +
                     player.wonderVP * weights.wonderVPBonus;
    double oppScore = opponent.coins * weights.coinValue +
                      opponent.cardCount * weights.economyPriority +
                      opponent.blueCards * weights.victoryPointValue +
                      opponent.greenCards * weights.sciencePriority +
                      opponent.redCards * weights.militaryPriority +
                      opponent.yellowCards * weights.economyPriority +
                      opponent.buildingVP * weights.victoryPointValue +
                      opponent.wonderVP * weights.wonderVPBonus;
    double militaryBonus = evaluateMilitaryPosition(state.militaryPosition, forPlayer1);
    myScore += militaryBonus * weights.militaryPriority;
    double scienceBonus = evaluateScientificProgress(player);
    myScore += scienceBonus * weights.sciencePriority;
    for (const auto& [res, qty] : player.permanentResources) {
        myScore += qty * weights.resourceValue;
    }
    for (const auto& [res, qty] : opponent.permanentResources) {
        oppScore += qty * weights.resourceValue;
    }
    return (myScore - oppScore) / (myScore + oppScore + 1.0);
}
double MCTS::evaluateResourceDenial(const MCTSGameState& state, const MCTSAction& action) const {
    const PlayerInfo& opponent = state.getOpponent();
    double denialValue = 0.0;
    if (opponent.brownCards < 2 || opponent.greyCards < 2) {
        if (action.cardColor == Models::ColorType::BROWN || 
            action.cardColor == Models::ColorType::GREY) {
            denialValue = 2.0;
        }
    }
    if (opponent.greenCards >= 2) {
        if (action.cardColor == Models::ColorType::GREEN) {
            denialValue = 3.0;
        }
    }
    return denialValue;
}
double MCTS::evaluateScientificProgress(const PlayerInfo& player) const {
    double scienceValue = 0.0;
    size_t symbolTypes = 0;
    for (const auto& [symbol, count] : player.scientificSymbols) {
        if (count > 0) symbolTypes++;
    }
    if (symbolTypes >= 6) {
        return 1000.0; 
    }
    scienceValue = symbolTypes * 5.0;
    return scienceValue;
}
double MCTS::evaluateMilitaryPosition(int position, bool forPlayer1) const {
    if (position <= 0) return forPlayer1 ? 1000.0 : -1000.0; 
    if (position >= 18) return forPlayer1 ? -1000.0 : 1000.0; 
    if (forPlayer1) {
        return (18 - position) * 0.5; 
    } else {
        return position * 0.5; 
    }
}
} 
