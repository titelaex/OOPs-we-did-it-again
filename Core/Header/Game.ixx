export module Core.Game;

import <memory>;
import Core.Board;
import Core.Node;
import Core.Player;

export namespace Core {
    void preparation();
    void awardMilitaryTokenIfPresent(Core::Player& receiver);
    void PrepareBoardCardPools();
    
    // Game phases
    void phaseI(Player& p1, Player& p2);
    void phaseII(Player& p1, Player& p2);
    void phaseIII(Player& p1, Player& p2);
    void wonderSelection(std::shared_ptr<Core::Player>& p1, std::shared_ptr<Core::Player>& p2);
}
