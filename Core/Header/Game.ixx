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
export namespace Core {
    class Game {
    public:
        static void preparation();
        static void PrepareBoardCardPools();
        static void initGame(bool& continueGame, bool& trainingMode, 
                            std::shared_ptr<Player>& p1, std::shared_ptr<Player>& p2,
                            IPlayerDecisionMaker*& p1Decisions, IPlayerDecisionMaker*& p2Decisions,
                            Playstyle& p1Playstyle, Playstyle& p2Playstyle);
        static void phaseI(Player& p1, Player& p2, 
                          IPlayerDecisionMaker* p1Decisions = nullptr, 
                          IPlayerDecisionMaker* p2Decisions = nullptr,
                          TrainingLogger* logger = nullptr);
        static void phaseII(Player& p1, Player& p2, 
                           IPlayerDecisionMaker* p1Decisions = nullptr, 
                           IPlayerDecisionMaker* p2Decisions = nullptr,
                           TrainingLogger* logger = nullptr);
        static void phaseIII(Player& p1, Player& p2, 
                            IPlayerDecisionMaker* p1Decisions = nullptr, 
                            IPlayerDecisionMaker* p2Decisions = nullptr,
                            TrainingLogger* logger = nullptr);
        static void runAgePhase(
            const std::vector<std::shared_ptr<Node>>& nodes,
            const std::string& phaseName,
            Player& p1,
            Player& p2,
            IPlayerDecisionMaker* p1Decisions,
            IPlayerDecisionMaker* p2Decisions,
            TrainingLogger* logger
        );
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
