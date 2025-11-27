module Models.Player;

import Models.Player;
import <iostream>;
import <algorithm>;
import <memory>;

using namespace Models;

uint8_t Player::totalCoins(std::tuple<uint8_t, uint8_t, uint8_t> coins) { return std::get<0>(coins) + std::get<1>(coins) * 3 + std::get<2>(coins) * 6; }

uint8_t Player::Points::totalVictoryPoints() { return m_militaryVictoryPoints + m_buildingVictoryPoints + m_wonderVictoryPoints + m_progressVictoryPoints; }

Player::Player(const uint8_t& id, const std::string& username) : kplayerId(id), kplayerUsername(username)
{
	m_playerPoints = { 0,0,0,0 };
}

void Player::addCard(std::unique_ptr<Card> card) { m_ownedCards.push_back(std::move(card)); }
void Player::addWonder(std::unique_ptr<Wonder> wonder) { m_ownedWonders.push_back(std::move(wonder)); }
void Player::addToken(const Token& token) { m_ownedTokens.push_back(token); }
void Player::addPermanentResource(ResourceType resourceType, uint8_t quantity) { m_ownedPermanentResources[resourceType] += quantity; }
void Player::addTradingResource(ResourceType resourceType, uint8_t quantity) { m_ownedTradingResources[resourceType] += quantity; }
void Player::setRemainingCoins(std::tuple<uint8_t,uint8_t,uint8_t> remainingCoins) { m_remainingCoins = remainingCoins; }
void Player::setPoints(Points playerPoints) { m_playerPoints = playerPoints; }
const uint8_t Player::getkPlayerId() const { return kplayerId; }
const std::string& Player::getPlayerUsername() const { return m_playerUsername; }
const std::vector<std::unique_ptr<Wonder>>& Player::getOwnedWonders() const { return m_ownedWonders; }
const std::vector<std::unique_ptr<Card>>& Player::getOwnedCards() const { return m_ownedCards; }
const std::vector<Token>& Player::getOwnedTokens() const { return m_ownedTokens; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedPermanentResources() const { return m_ownedPermanentResources; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedTradingResources() const { return m_ownedTradingResources; }
std::tuple<uint8_t,uint8_t,uint8_t> Player::getRemainingCoins() const { return m_remainingCoins; }
Player::Points Player::getPoints() const { return m_playerPoints; }

//void Player::addCoins(uint8_t coinsToAdd)
//{
//	auto& [ones, threes, sixes] = m_remainingCoins;
//	while (coinsToAdd >= 6) { ++sixes; coinsToAdd -= 6; }
//	while (coinsToAdd >= 3) { ++threes; coinsToAdd -= 3; }
//	while (coinsToAdd > 0) { ++ones; --coinsToAdd; }
//}
//
//void Player::subtractCoins(uint8_t coinsToSubtract)
//{
//	auto& [ones, threes, sixes] = m_remainingCoins;
//	uint8_t total = totalCoins(m_remainingCoins);
//	if (coinsToSubtract > total) { ones = threes = sixes = 0; return; }
//	while (coinsToSubtract >= 6 && sixes > 0) { --sixes; coinsToSubtract -= 6; }
//	while (coinsToSubtract >= 3 && threes > 0) { --threes; coinsToSubtract -= 3; }
//	while (coinsToSubtract > 0 && ones > 0) { --ones; --coinsToSubtract; }
//	while (coinsToSubtract > 0) {
//		if (sixes > 0) { --sixes; addCoins(6 - coinsToSubtract); coinsToSubtract = 0; }
//		else if (threes > 0) { --threes; addCoins(3 - coinsToSubtract); coinsToSubtract = 0; }
//		else break;
//	}
//}
//

//
//void Player::burnCard(Card& card)
//{
//	uint8_t yellowCards = countYellowCards();
//	uint8_t coinsEarned = 2 + yellowCards;
//	addCoins(coinsEarned);
//
//	auto it = std::find_if(m_ownedCards.begin(), m_ownedCards.end(), [&](const std::unique_ptr<Card>& p) { return p && p.get() == &card; });
//	if (it != m_ownedCards.end()) {
//		(*it)->SetIsVisible(false);
//		std::cout << "Card \"" << (*it)->GetName() << "\" discarded. Player \"" << getPlayerUsername() << "\" gains " << static_cast<int>(coinsEarned) << " coins.\n";
//		m_ownedCards.erase(it);
//	} else {
//		card.SetIsVisible(false);
//		std::cout << "Card \"" << card.GetName() << "\" discarded. Player \"" << getPlayerUsername() << "\" gains " << static_cast<int>(coinsEarned) << " coins.\n";
//	}
//}
