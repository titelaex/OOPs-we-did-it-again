module Models.Wonder;
import <random>;
import <algorithm>;
import <iterator>;
import Models.Wonder;
import Models.Player; // ensure Player available for action methods

using namespace Models;

void Wonder::receiveMoneyAction(Player& player) { player.addCoins(kplayerReceivesMoney); }
void Wonder::opponentLosesMoneyAction(Player& opponent) { opponent.subtractCoins(static_cast<uint8_t>(-kopponentLosesMoney)); }
void Wonder::playSecondTurnAction(Player& /*player*/) { /* game engine hook */ }

std::vector<Token> Wonder::randomTokenSelector(std::vector<Token>& discardedTokens)
{
	const uint8_t tokensToSelect = 3;
	if (discardedTokens.size() <= tokensToSelect) return discardedTokens;
	std::vector<Token> selectedTokens;
	std::random_device seed; std::mt19937 generator(seed());
	std::sample(discardedTokens.begin(), discardedTokens.end(), std::back_inserter(selectedTokens), tokensToSelect, generator);
	return selectedTokens;
}

void Wonder::m_drawProgressTokenAction(std::vector<Token>& discardedTokens)
{
	auto selectedTokens = randomTokenSelector(discardedTokens);
	// UI selection placeholder
}

void Wonder::m_chooseAndConstructBuildingAction(const std::vector<Card>& /*discardedCards*/) { /* UI + free build logic */ }
void Wonder::m_discardBrownCardFromOpponentAction(Player& /*opponent*/) { /* UI discard logic */ }
