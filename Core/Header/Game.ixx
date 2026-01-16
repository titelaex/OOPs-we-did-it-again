export module Core.Game;
import <memory>;
import <vector>;
import <string>;
import Core.Board;
import Core.Node;
import Core.Player;
import Models.Card;
import Core.PlayerDecisionMaker;
import Core.TrainingLogger;
import Core.AIConfig;
import Core.IGameListener;
export namespace Core {
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
    };
}
