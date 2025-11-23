#include <vector>
#include <string>
#include <random>

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

void m_receiveMoneyAction(class Player& player)
{

}
void m_opponentLosesMoneyAction(class Player& opponent)
{

}
void m_playSecondTurnAction(class Player& player)
{

}
std::vector<Token> randomTokenSelector(std::vector<Token>& discardedTokens)
{
	const uint8_t tokensToSelect = 3;
	if (discardedTokens.size() <= tokensToSelect) return discardedTokens;
	std::vector<Token> selectedTokens;
	std::random_device seed; std::mt19937 generator(seed());
	std::sample(discardedTokens.begin(), discardedTokens.end(), std::back_inserter(selectedTokens), tokensToSelect, generator);
	return selectedTokens;
}

void m_drawProgressTokenAction(std::vector<Token>& discardedTokens)
{
	auto selectedTokens = randomTokenSelector(discardedTokens);
	// UI selection placeholder
}

void m_chooseAndConstructBuildingAction(const std::vector<Card>& discardedCards)
{

}
void m_discardBrownCardFromOpponentAction(class Player& opponent)
{

}

int main()
{
	
}