export module Core.Game;
import <memory>;
import <vector>;
import <string>;
import <optional>;
import <unordered_map>;
import Core.Board;
import Core.Node;
import Core.Player;
import Models.Card;
import Models.Wonder;
import Models.ResourceType;
import Core.PlayerDecisionMaker;
import Core.TrainingLogger;
import Core.AIConfig;
import Core.IGameListener;
<<<<<<< HEAD

=======
>>>>>>> parent of 5f22777 (Solved all Core Game issues and implemented ranges and views for tryWidthdraw)
export namespace Core {
    export struct WonderTradeCostLine {
        Models::ResourceType resource = Models::ResourceType::NO_RESOURCE;
        uint8_t amount = 0;
        uint8_t costPerUnit = 0;
        uint8_t totalCost = 0;
        bool discounted = false;
    };

    export struct WonderTradeCostBreakdown {
        bool canAfford = false;
        uint8_t availableCoins = 0;
        uint8_t totalCost = 0;
        bool architectureTokenApplied = false;
        std::vector<WonderTradeCostLine> lines;
    };

    class Game {
    public:
        static GameEventNotifier& getNotifier();
        static void preparation();
        static void PrepareBoardCardPools();
        static void initGame();
        
        static void playAllPhases(Player& p1, Player& p2,
                                 IPlayerDecisionMaker& p1Decisions,
                                 IPlayerDecisionMaker& p2Decisions,
                                 TrainingLogger* logger = nullptr);
        
        static void wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2, 
                                    IPlayerDecisionMaker* p1Decisions = nullptr, IPlayerDecisionMaker* p2Decisions = nullptr);
        static void debugWonders(const std::vector<std::unique_ptr<Models::Card>>& pool);
        static void awardMilitaryTokenIfPresent(Player& receiver);
        static void movePawn(int steps);
        static void displayPlayerHands(const Player& p1, const Player& p2);
        static void displayTurnStatus(const Player& p1, const Player& p2);
        static void announceVictory(int winner, const std::string& victoryType, const Player& p1, const Player& p2);
<<<<<<< HEAD

        // Backend-owned helper: after a node is emptied, reveal newly-unlocked parents and notify listeners.
        static void updateTreeAfterPick(int age, int emptiedNodeIndex);

        // Apply an action on a specific tree node for the current player.
        // action: 0=build, 1=sell, 2=wonder
        // wonderIndex: used only when action==2; otherwise ignored.
        // Returns true if the action succeeds and the node is emptied.
        static bool applyTreeCardAction(int age, int nodeIndex, int action, std::optional<size_t> wonderIndex = std::nullopt);

        // Computes how many coins the current player would need to pay (trade) to build `wonder`.
        // Mirrors the rule logic used by `Player::canAffordWonder/payForWonder`.
        static WonderTradeCostBreakdown computeWonderTradeCost(const Player& cur,
            const Models::Wonder& wonder,
            const Player& opp);
=======
>>>>>>> parent of 5f22777 (Solved all Core Game issues and implemented ranges and views for tryWidthdraw)
    };
}
