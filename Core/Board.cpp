module Core.Board;

import Core.Node;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;

#include <algorithm>

namespace Core {
    std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
    std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
    std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards; // excluding guilds
    std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
    std::vector<std::unique_ptr<Models::Card>> unusedWonders;

    std::vector<std::unique_ptr<Node>> age1Nodes;
    std::vector<std::unique_ptr<Node>> age2Nodes;
    std::vector<std::unique_ptr<Node>> age3Nodes;

    void SetupCardPools()
    {
        unusedAgeOneCards.reserve(23);
        unusedAgeTwoCards.reserve(23);
        unusedAgeThreeCards.reserve(20);
        unusedGuildCards.reserve(7);
        unusedWonders.reserve(12);
    }
}
