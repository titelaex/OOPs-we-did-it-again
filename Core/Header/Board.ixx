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
import Models.Bank;

export namespace Core {

    export class Board{
    private:
        Models::Bank bank{};
        std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
        std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
        std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards;
        std::vector<std::unique_ptr<Models::Card>> unusedGuildCards;
        std::vector<std::unique_ptr<Models::Card>> unusedWonders;

		std::vector<std::unique_ptr<Models::Card>> discardedCards;

        std::bitset<19> pawnTrack;
        uint8_t pawnPos;
        std::vector<Models::Token> progressTokens;
        std::vector<Models::Token> militaryTokens;

        Models::Bank& getBank();
        void SetupCardPools();
        void displayBoard();
	    void displayEntireBoard();

        std::vector<std::unique_ptr<Node>> age1Nodes;
        std::vector<std::unique_ptr<Node>> age2Nodes;
        std::vector<std::unique_ptr<Node>> age3Nodes;

    private:
        Board();
        Board(const Board&) = delete;
        Board& operator=(const Board&) = delete;
    public:
        static Board& getInstance() {
            static Board instance;
            return instance;
        }
    };
}
