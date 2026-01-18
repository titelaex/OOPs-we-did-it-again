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
import Models.ColorType;
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
        static void awardMilitaryTokenIfPresent(Player& receiver, Player& opponent);
        static void movePawn(int steps);
        static void displayPlayerHands(const Player& p1, const Player& p2);
        static void displayTurnStatus(const Player& p1, const Player& p2);
        static void announceVictory(int winner, const std::string& victoryType, const Player& p1, const Player& p2);
        static void handleOpponentCardDiscard(Player& cardOwner, Player& discardingPlayer, 
                                              Models::ColorType color, 
                                              IPlayerDecisionMaker& decisionMaker);
    };
}
