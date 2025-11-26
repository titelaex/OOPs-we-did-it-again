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

    void displayUnusedPools()
    {
		  std::cout << "Age I unused: " << Core::unusedAgeOneCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedAgeOneCards.size(); ++i) {
            auto& p = Core::unusedAgeOneCards[i];
            if (p) {
                std::cout << "[AgeI] card " << i << ":\n";
                p->displayCardInfo();
            }
        }

        std::cout << "Age II unused: " << Core::unusedAgeTwoCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedAgeTwoCards.size(); ++i) {
            auto& p = Core::unusedAgeTwoCards[i];
            if (p) {
                std::cout << "[AgeII] card " << i << ":\n";
                p->displayCardInfo();
            }
        }

        std::cout << "Age III unused: " << Core::unusedAgeThreeCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedAgeThreeCards.size(); ++i) {
            auto& p = Core::unusedAgeThreeCards[i];
            if (p) {
                std::cout << "[AgeIII] card " << i << ":\n";
                p->displayCardInfo();
            }
        }

        std::cout << "Guild unused: " << Core::unusedGuildCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedGuildCards.size(); ++i) {
            auto& p = Core::unusedGuildCards[i];
            if (p) {
                std::cout << "[Guild] card " << i << ":\n";
                p->displayCardInfo();
            }
        }

        std::cout << "Wonders unused: " << Core::unusedWonders.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedWonders.size(); ++i) {
            auto& p = Core::unusedWonders[i];
            if (p) {
                std::cout << "[Wonder] card " << i << ":\n";
                p->displayCardInfo();
            }
        }
    }
	void displayEntireBoard() {
		std::cout << "=== Board State ===\n";
		displayBoard();
		std::cout << "===================\n";
        std::cout << "--- UNUSED POOLS ---\n";

      

        std::cout << "--- Age I Nodes (" << Core::age1Nodes.size() << " nodes) ---\n";
        for (size_t i = 0; i < Core::age1Nodes.size(); ++i) {
            auto& n = Core::age1Nodes[i];
            if (!n) continue;
            Models::Card* c = n->getCard();
            if (!c) continue;
            std::cout << "Node[" << i << "]:\n";
            c->displayCardInfo();
            auto p1 = n->getParent1();
            auto p2 = n->getParent2();
            std::cout << " Parents=(" << (p1 ? "1" : " ") << "," << (p2 ? "1" : " ") << ")\n";
        }

        std::cout << "--- Age II Nodes (" << Core::age2Nodes.size() << " nodes) ---\n";
        for (size_t i = 0; i < Core::age2Nodes.size(); ++i) {
            auto& n = Core::age2Nodes[i];
            if (!n) continue;
            Models::Card* c = n->getCard();
            if (!c) continue;
            std::cout << "Node[" << i << "]:\n";
            c->displayCardInfo();
            auto p1 = n->getParent1();
            auto p2 = n->getParent2();
            std::cout << " Parents=(" << (p1 ? "1" : " ") << "," << (p2 ? "1" : " ") << ")\n";
        }

        std::cout << "--- Age III Nodes (" << Core::age3Nodes.size() << " nodes) ---\n";
        for (size_t i = 0; i < Core::age3Nodes.size(); ++i) {
            auto& n = Core::age3Nodes[i];
            if (!n) continue;
            Models::Card* c = n->getCard();
            if (!c) continue;
            std::cout << "Node[" << i << "]:\n";
            c->displayCardInfo();
            auto p1 = n->getParent1();
            auto p2 = n->getParent2();
            std::cout << " Parents=(" << (p1 ? "1" : " ") << "," << (p2 ? "1" : " ") << ")\n";
        }
	}
}