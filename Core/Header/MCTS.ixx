export module Core.MCTS;
import <vector>;
import <memory>;
import <random>;
import <cstdint>;
import <unordered_map>;
import <string>;
import Models.ResourceType;
import Models.ScientificSymbolType;
import Models.ColorType;
import Core.AIConfig;
export namespace Core {
    export struct CardInfo {
        std::string name;
        Models::ColorType color = Models::ColorType::NO_COLOR;
        uint8_t victoryPoints = 0;
        uint8_t shieldPoints = 0;
        std::unordered_map<Models::ResourceType, uint8_t> resourcesProduction;
        std::unordered_map<Models::ResourceType, uint8_t> resourceCost;
        bool hasScientificSymbol = false;
        Models::ScientificSymbolType scientificSymbol = Models::ScientificSymbolType::NO_SYMBOL;
        bool isConstructed = false;
    };
    export struct PlayerInfo {
        uint8_t coins = 7;
        uint8_t cardCount = 0;
        uint8_t wonderCount = 0;
        uint8_t tokenCount = 0;
        std::vector<CardInfo> ownedCards;
        std::vector<CardInfo> ownedWonders;
        std::unordered_map<Models::ResourceType, uint8_t> permanentResources;
        std::unordered_map<Models::ResourceType, uint8_t> tradingResources;
        std::unordered_map<Models::ScientificSymbolType, uint8_t> scientificSymbols;
        uint8_t militaryVP = 0;
        uint8_t buildingVP = 0;
        uint8_t wonderVP = 0;
        uint8_t progressVP = 0;
        uint8_t brownCards = 0;
        uint8_t greyCards = 0;
        uint8_t blueCards = 0;
        uint8_t redCards = 0;
        uint8_t greenCards = 0;
        uint8_t yellowCards = 0;
        uint8_t purpleCards = 0;
    };
    export struct MCTSAction {
        size_t cardNodeIndex = 0;
        int actionType = 0;
        size_t wonderIndex = 0;
        std::string cardName;
        Models::ColorType cardColor = Models::ColorType::NO_COLOR;
        uint8_t expectedVP = 0;
        bool blocksOpponent = false;
        bool operator==(const MCTSAction& other) const {
            return cardNodeIndex == other.cardNodeIndex && 
                   actionType == other.actionType && 
                   wonderIndex == other.wonderIndex;
        }
    };
    export struct MCTSGameState {
        PlayerInfo player1;
        PlayerInfo player2;
        int militaryPosition = 9;
        int currentPhase = 1;
        bool player1Turn = true;
        int roundsRemaining = 20;
        bool gameOver = false;
        int winner = -1;
        std::vector<MCTSAction> availableActions;
        std::string serializedState;
        MCTSGameState clone() const {
            MCTSGameState copy = *this;
            copy.availableActions = this->availableActions;
            copy.serializedState = this->serializedState;
            copy.player1 = this->player1;
            copy.player2 = this->player2;
            return copy;
        }
        bool isTerminal() const { return gameOver || roundsRemaining <= 0; }
        const PlayerInfo& getCurrentPlayer() const {
            return player1Turn ? player1 : player2;
        }
        const PlayerInfo& getOpponent() const {
            return player1Turn ? player2 : player1;
        }
        PlayerInfo& getCurrentPlayerMut() {
            return player1Turn ? player1 : player2;
        }
        PlayerInfo& getOpponentMut() {
            return player1Turn ? player2 : player1;
        }
    };
    export class MCTSNode {
    public:
        MCTSNode(const MCTSGameState& state, MCTSNode* parent = nullptr, const MCTSAction& action = MCTSAction{});
        ~MCTSNode() = default;
        MCTSNode* select(double explorationConstant);
        MCTSNode* expand();
        double simulate(std::mt19937& rng, int maxDepth);
        void backpropagate(double reward);
        double getUCB1Score(double explorationConstant) const;
        const MCTSAction& getAction() const { return m_action; }
        const MCTSGameState& getState() const { return m_state; }
        int getVisits() const { return m_visits; }
        double getValue() const { return m_totalValue; }
        bool isFullyExpanded() const;
        bool isLeaf() const { return m_children.empty(); }
        const std::vector<std::unique_ptr<MCTSNode>>& getChildren() const { return m_children; }
    private:
        MCTSGameState m_state;
        MCTSAction m_action;
        MCTSNode* m_parent;
        std::vector<std::unique_ptr<MCTSNode>> m_children;
        int m_visits = 0;
        double m_totalValue = 0.0;
        size_t m_untriedActionsIndex = 0;
    };
    export class MCTS {
    public:
        MCTS(int iterations = 1000, double explorationConstant = 1.414, int maxSimulationDepth = 50, 
             Playstyle playstyle = Playstyle::BRITNEY);
        MCTSAction search(const MCTSGameState& rootState);
        void setIterations(int iterations) { m_iterations = iterations; }
        void setExplorationConstant(double c) { m_explorationConstant = c; }
        void setMaxDepth(int depth) { m_maxSimulationDepth = depth; }
        void setPlaystyle(Playstyle style) { m_playstyle = style; }
        Playstyle getPlaystyle() const { return m_playstyle; }
        static std::vector<MCTSAction> getLegalActions(const MCTSGameState& state, int currentPhase);
        static void applyAction(MCTSGameState& state, const MCTSAction& action);
        static bool isTerminal(const MCTSGameState& state);
        static int getWinner(const MCTSGameState& state);
        static MCTSGameState cloneState(const MCTSGameState& state);
        static MCTSGameState captureGameState(int currentPhase, bool isPlayer1Turn);
        static void saveCurrentState(MCTSGameState& mctsState);
        static void restoreState(const MCTSGameState& mctsState);
    private:
        int m_iterations;
        double m_explorationConstant;
        int m_maxSimulationDepth;
        std::mt19937 m_rng;
        Playstyle m_playstyle;
        double evaluateState(const MCTSGameState& state, bool forPlayer1) const;
        double evaluateResourceDenial(const MCTSGameState& state, const MCTSAction& action) const;
        double evaluateScientificProgress(const PlayerInfo& player) const;
        double evaluateMilitaryPosition(int position, bool forPlayer1) const;
    };
} 
