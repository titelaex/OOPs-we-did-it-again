export module Core.Game;

import Core.Board;
import Core.Node;
import Core.Player;

export namespace Core {
    void preparation();
    void awardMilitaryTokenIfPresent(Core::Player& receiver);
    export void PrepareBoardCardPools();
}
