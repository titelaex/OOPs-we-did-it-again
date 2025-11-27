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



        }
        catch (const std::exception& ex) {
            std::cerr << "Preparation exception: " << ex.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown exception during preparation" << std::endl;
        }
    }
}