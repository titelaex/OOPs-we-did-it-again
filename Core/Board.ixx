export module Core.Board;

import <vector>;
import <memory>;
import Core.Node;
import Models.Card;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;

export namespace Core {
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedWonders;

    export void SetupCardPools();

    export extern std::vector<std::unique_ptr<Node>> age1Nodes;
    export extern std::vector<std::unique_ptr<Node>> age2Nodes;
    export extern std::vector<std::unique_ptr<Node>> age3Nodes;

    export void BuildAge1Tree(uint32_t seed);
    export void BuildAge2Tree(uint32_t seed);
    export void BuildAge3Tree(uint32_t seed);
}
