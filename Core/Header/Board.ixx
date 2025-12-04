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

    export class Bank{
    public:
   
    std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
    std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
    std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards;
    std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
    std::vector<std::unique_ptr<Models::Card>> unusedWonders;

    std::bitset<19> pawnTrack;
    uint8_t pawnPos;
    std::vector<Models::Token> progressTokens;
    std::vector<Models::Token> militaryTokens;

    
    SetupCardPools();
    displayBoard();
	displayEntireBoard();

    std::vector<std::unique_ptr<Node>> age1Nodes;
    std::vector<std::unique_ptr<Node>> age2Nodes;
    std::vector<std::unique_ptr<Node>> age3Nodes;
    };
}
