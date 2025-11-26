export module Core.Board;

import <bitset>;
import <vector>;
import <memory>;
import Core.Node;
import Models.Card;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Token;

export namespace Core {
   
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
    export extern std::vector<std::unique_ptr<Models::Card>> unusedWonders;

    export extern std::bitset<19> pawnTrack;
    export extern uint8_t pawnPos;
    export extern std::vector<Models::Token> progressTokens;
    export extern std::vector<bool> militaryTokens;

    
    export void SetupCardPools();
    export void displayBoard();
	export void displayEntireBoard();

    export extern std::vector<std::unique_ptr<Node>> age1Nodes;
    export extern std::vector<std::unique_ptr<Node>> age2Nodes;
    export extern std::vector<std::unique_ptr<Node>> age3Nodes;
}
