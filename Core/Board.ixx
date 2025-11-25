export module Core.Board;

import <vector>;
import <memory>;
import Core.Node;
import Models.Card;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;

export namespace Core {
    // Unused card pools stored as owning unique_ptrs so cards can be moved into node graphs without copying.
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedWonders;

    // Reserve capacity for known game counts (called during preparation)
    export void SetupCardPools();

    // The built graphs are stored as vectors of owning Nodes. Each Node owns a single Card instance.
    // These vectors are populated during preparation and then used by the game logic.
    export extern std::vector<std::unique_ptr<Node>> age1Nodes;
    export extern std::vector<std::unique_ptr<Node>> age2Nodes;
    export extern std::vector<std::unique_ptr<Node>> age3Nodes;
}
