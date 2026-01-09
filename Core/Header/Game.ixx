export module Core.Game;

import <memory>;
import <vector>;
import Core.Board;
import Core.Node;
import Core.Player;
import Models.Card;

// Forward declare the interface (defined in PlayerDecisionMaker.h)
namespace Core {
    class IPlayerDecisionMaker;
}

export namespace Core {
    void preparation();
    void awardMilitaryTokenIfPresent(Core::Player& receiver);
    void PrepareBoardCardPools();
    
    // Game phases with decision maker support
    void phaseI(Player& p1, Player& p2, Core::IPlayerDecisionMaker* p1Decisions = nullptr, Core::IPlayerDecisionMaker* p2Decisions = nullptr);
    void phaseII(Player& p1, Player& p2, Core::IPlayerDecisionMaker* p1Decisions = nullptr, Core::IPlayerDecisionMaker* p2Decisions = nullptr);
    void phaseIII(Player& p1, Player& p2, Core::IPlayerDecisionMaker* p1Decisions = nullptr, Core::IPlayerDecisionMaker* p2Decisions = nullptr);
    void wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2);
    void debugWonders(const std::vector<std::unique_ptr<Models::Card>>& pool);
}
