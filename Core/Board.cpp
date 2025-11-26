module Core.Board;
import Core.Node;
import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Token;
import Models.Card;

import <vector>;
import <algorithm>;
import <bitset>;
import <iostream>;

namespace Core {
	std::vector<std::unique_ptr<Models::Card>> unusedAgeOneCards;
	std::vector<std::unique_ptr<Models::Card>> unusedAgeTwoCards;
	std::vector<std::unique_ptr<Models::Card>> unusedAgeThreeCards;
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

	std::bitset<19> pawnTrack = 512;
	uint8_t pawnPos;
	std::vector<Models::Token> progressTokens;
	std::vector<bool> militaryTokens;

	void displayBoard() {

		std::cout << "Progress Tokens: ";
		for (const auto& t : progressTokens) std::cout << "[" << t << "] ";
		std::cout << "\n\n";

	
		std::cout << "C1 |";
		for (int i = 0; i < 19; ++i) {
			if (pawnTrack.test(i))
				std::cout << " ! ";
			else
				std::cout << " _ ";

			if (((i + 1) % 3 == 0 && i < 9) || (i % 3 == 0 && i >= 9))
				std::cout << "|";
		}
		std::cout << " C2\n\n";

		
		std::cout << "Military Tokens: ";
		for (const auto& m : militaryTokens) std::cout << "[" << m << "] ";
		std::cout << "\n";
	}


}