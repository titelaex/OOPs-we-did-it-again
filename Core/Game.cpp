module Core.Game;

import <vector>;
import <string>;
import <random>;
import <iostream>;
import <algorithm>;

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
import Models.Token; // ensures loadTokensFromCSV is available
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
std::vector<Models::Token> startGameTokens(std::vector<Models::Token>& allTokens)
{
    constexpr size_t kSelectCount = 5;
    if (allTokens.size() <= kSelectCount) return allTokens;
    std::vector<Models::Token> selected; selected.reserve(kSelectCount);
    std::random_device rd; std::mt19937 gen(rd());
    std::sample(allTokens.begin(), allTokens.end(), std::back_inserter(selected), kSelectCount, gen);
    return selected;
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


namespace Core {
    void preparation()
    {
        try {
            Core::PrepareBoardCardPools();
            
            auto allTokens = Models::loadTokensFromCSV("Tokens.csv");
            Core::progressTokens = startGameTokens(allTokens);
        }
        catch (const std::exception& ex) {
            std::cerr << "Preparation exception: " << ex.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown exception during preparation" << std::endl;
        }
    }
}