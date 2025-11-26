
module Core.Game;

import <vector>;
import <string>;
import <random>;
import <iostream>;

import Core.Preparation;
import Core.Board;
import Core.Node;

import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.ResourceType;
import Models.ScientificSymbolType;
import Models.LinkingSymbolType;
import Models.CoinWorthType;
import Models.ColorType;
import Models.Age;
import Models.TradeRuleType;
import Models.Token;
import Models.Card;

void m_receiveMoneyAction(class Player& player)
{

}
void m_opponentLosesMoneyAction(class Player& opponent)
{

}
void m_playSecondTurnAction(class Player& player)
{

}
std::vector<Models::Token> randomTokenSelector(std::vector<Models::Token>& discardedTokens)
{
    const uint8_t tokensToSelect = 3;
    if (discardedTokens.size() <= tokensToSelect) return discardedTokens;
    std::vector<Models::Token> selectedTokens;
    std::random_device seed; std::mt19937 generator(seed());
    std::sample(discardedTokens.begin(), discardedTokens.end(), std::back_inserter(selectedTokens), tokensToSelect, generator);
    return selectedTokens;
}

void m_drawProgressTokenAction(std::vector<Models::Token>& discardedTokens)
{
    auto selectedTokens = randomTokenSelector(discardedTokens);
    // UI selection placeholder
}

void m_chooseAndConstructBuildingAction(const std::vector<Models::AgeCard>& discardedCards)
{

}
void m_discardCardFromOpponentAction(class Player& opponent, Models::ColorType color)
{

}

void movePawn(int steps) {
    Core::pawnTrack.reset();
    Core::pawnPos = std::clamp(Core::pawnPos + steps, 0, 18);
    Core::pawnTrack.set(Core::pawnPos);
}

// Preparation helper used by tests: this calls the preparation routine and then prints
// the contents of the unused pools and the constructed age node graphs so you can
// visually verify that cards were loaded, shuffled and moved into nodes correctly.
namespace Core {
void preparation()
{
    try {
        Core::PrepareBoardCardPools();

        std::cout << "--- UNUSED POOLS ---\n";

        std::cout << "Age I unused: " << Core::unusedAgeOneCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedAgeOneCards.size(); ++i) {
            auto& p = Core::unusedAgeOneCards[i];
            if (p) {
                std::cout << "[AgeI] card " << i << ":\n";
                p->displayCard();
            }
        }

        std::cout << "Age II unused: " << Core::unusedAgeTwoCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedAgeTwoCards.size(); ++i) {
            auto& p = Core::unusedAgeTwoCards[i];
            if (p) {
                std::cout << "[AgeII] card " << i << ":\n";
                p->displayCard();
            }
        }

        std::cout << "Age III unused: " << Core::unusedAgeThreeCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedAgeThreeCards.size(); ++i) {
            auto& p = Core::unusedAgeThreeCards[i];
            if (p) {
                std::cout << "[AgeIII] card " << i << ":\n";
                p->displayCard();
            }
        }

        std::cout << "Guild unused: " << Core::unusedGuildCards.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedGuildCards.size(); ++i) {
            auto& p = Core::unusedGuildCards[i];
            if (p) {
                std::cout << "[Guild] card " << i << ":\n";
                p->displayCard();
            }
        }

        std::cout << "Wonders unused: " << Core::unusedWonders.size() << " cards\n";
        for (size_t i = 0; i < Core::unusedWonders.size(); ++i) {
            auto& p = Core::unusedWonders[i];
            if (p) {
                std::cout << "[Wonder] card " << i << ":\n";
                p->displayCard();
            }
        }

        std::cout << "--- Age I Nodes (" << Core::age1Nodes.size() << " nodes) ---\n";
        for (size_t i = 0; i < Core::age1Nodes.size(); ++i) {
            auto& n = Core::age1Nodes[i];
            if (!n) continue;
            Models::Card* c = n->getCard();
            if (!c) continue;
            std::cout << "Node[" << i << "]:\n";
            c->displayCard();
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
            c->displayCard();
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
            c->displayCard();
            auto p1 = n->getParent1();
            auto p2 = n->getParent2();
            std::cout << " Parents=(" << (p1 ? "1" : " ") << "," << (p2 ? "1" : " ") << ")\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Preparation exception: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception during preparation" << std::endl;
    }
}
} // namespace Core