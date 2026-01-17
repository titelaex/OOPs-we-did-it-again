module Models.Player;

import Models.Player;
import Models.ColorType;
import <iostream>;
import <algorithm>;
import <memory>;

using namespace Models;

uint8_t Player::totalCoins(const std::tuple<uint8_t, uint8_t, uint8_t>& coins) { return std::get<0>(coins) + std::get<1>(coins) *3 + std::get<2>(coins) *6; }

uint8_t Player::Points::totalVictoryPoints() const { return m_militaryVictoryPoints + m_buildingVictoryPoints + m_wonderVictoryPoints + m_progressVictoryPoints + m_coinVictoryPoints; }

Player::Player(const uint8_t& id, const std::string& username) : playerId(id), m_playerUsername(username)
{
	m_playerPoints = {0,0,0,0 };
}

void Player::addCard(std::unique_ptr<Card> card) { 
	if (!card) return;
	uint8_t cardVP = card->getVictoryPoints();
	if (cardVP > 0) {
		Models::ColorType color = card->getColor();
		if (color == Models::ColorType::BLUE || 
		    color == Models::ColorType::GREEN || 
		    color == Models::ColorType::YELLOW || 
		    color == Models::ColorType::PURPLE) {
			m_playerPoints.m_buildingVictoryPoints += cardVP;
		}
	}
	m_ownedCards.push_back(std::move(card)); 
}

void Player::addWonder(std::unique_ptr<Wonder> wonder) { 
	if (!wonder) return;
	m_ownedWonders.push_back(std::move(wonder)); 
}

void Player::addToken(std::unique_ptr<Token> token) { 
	if (!token) return;
	uint8_t tokenVP = token->getVictoryPoints();
	if (tokenVP > 0) {
		m_playerPoints.m_progressVictoryPoints += tokenVP;
	}
	m_ownedTokens.push_back(std::move(token)); 
}

void Player::addPermanentResource(const ResourceType& resourceType, const uint8_t& quantity) { m_ownedPermanentResources[resourceType] += quantity; }
void Player::addTradingResource(const ResourceType& resourceType, const uint8_t& quantity) { m_ownedTradingResources[resourceType] += quantity; }
void Models::Player::setPlayerUsername(const std::string& username) { m_playerUsername = username; }
void Player::setRemainingCoins(const std::tuple<uint8_t,uint8_t,uint8_t>& remainingCoins) { m_remainingCoins = remainingCoins; }
void Player::setPoints(const Points& playerPoints) { m_playerPoints = playerPoints; }
void Player::addScientificSymbol(const ScientificSymbolType& symbol, const uint8_t& quantity) { m_ownedScientificSymbols[symbol] += quantity; }
const uint8_t Player::getkPlayerId() const { return playerId; }
const std::string& Player::getPlayerUsername() const { return m_playerUsername; }
std::vector<std::unique_ptr<Models::Wonder>>& Models::Player::getOwnedWonders() {return m_ownedWonders;}
const std::vector<std::unique_ptr<Card>>& Player::getOwnedCards() const { return m_ownedCards; }
const std::vector<std::unique_ptr<Token>>& Player::getOwnedTokens() const { return m_ownedTokens; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedPermanentResources() const { return m_ownedPermanentResources; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedTradingResources() const { return m_ownedTradingResources; }
const std::tuple<uint8_t,uint8_t,uint8_t> Player::getRemainingCoins() const { return m_remainingCoins; }
const std::unordered_map<ScientificSymbolType, uint8_t>& Player::getOwnedScientificSymbols() const { return m_ownedScientificSymbols; }
const Player::Points& Player::getPoints() const { return m_playerPoints; }
const std::unordered_map<TradeRuleType, bool>& Player::getTradeRules() const { return m_tradeRules; }
void Player::setTokensOwned(const std::bitset<10>& tokensOwned) { m_tokensOwned = tokensOwned; }
const std::bitset<10>& Player::getTokensOwned() const { return m_tokensOwned; }

void Player::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) {
 for (const auto& kv : tradeRules) {
 if (kv.second) {
 m_tradeRules[kv.first] = true;
 } else {
 if (m_tradeRules.find(kv.first) == m_tradeRules.end()) {
 m_tradeRules[kv.first] = false;
 }
 }
 }
}

std::unique_ptr<Card> Player::removeOwnedCardAt(size_t idx) {
 if (idx >= m_ownedCards.size()) return nullptr;
 std::unique_ptr<Card> out = std::move(m_ownedCards[idx]);
 m_ownedCards.erase(m_ownedCards.begin() + idx);
 return out;
}

uint8_t Player::getTotalVictoryPoints() const {
 return m_playerPoints.totalVictoryPoints();
}

uint8_t Player::getBlueBuildingVictoryPoints() const {
 uint8_t blueVP = 0;
 for (const auto& card : m_ownedCards) {
 if (card && card->getColor() == ColorType::BLUE) {
 blueVP += card->getVictoryPoints();
 }
 }
 return blueVP;
}

bool Player::hasToken(TokenIndex tokenIndex) const {
 return m_tokensOwned[static_cast<size_t>(tokenIndex)];
}

uint8_t Player::getTokenVictoryBonus() const {
 uint8_t bonus = 0;
 if (hasToken(TokenIndex::PHILOSOPHY)) {
 bonus += 7;
 }
 if (hasToken(TokenIndex::MATHEMATICS)) {
 const auto& tokens = getOwnedTokens();
 bonus += 3 * static_cast<uint8_t>(tokens.size());
 }
 return bonus;
}
