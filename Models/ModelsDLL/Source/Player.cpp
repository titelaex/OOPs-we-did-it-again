module Models.Player;

import Models.Player;
import <iostream>;
import <algorithm>;
import <memory>;

using namespace Models;

uint8_t Player::totalCoins(const std::tuple<uint8_t, uint8_t, uint8_t>& coins) { return std::get<0>(coins) + std::get<1>(coins) * 3 + std::get<2>(coins) * 6; }

uint8_t Player::Points::totalVictoryPoints() { return m_militaryVictoryPoints + m_buildingVictoryPoints + m_wonderVictoryPoints + m_progressVictoryPoints; }

Player::Player(const uint8_t& id, const std::string& username) : kplayerId(id), m_playerUsername(username)
{
	m_playerPoints = { 0,0,0,0 };
}

void Player::addCard(const std::unique_ptr<Card>& card) { m_ownedCards.push_back(std::move(const_cast<std::unique_ptr<Card>&>(card))); }
void Player::addWonder(const std::unique_ptr<Wonder>& wonder) { m_ownedWonders.push_back(std::move(const_cast<std::unique_ptr<Wonder>&>(wonder))); }
void Player::addToken(const std::unique_ptr<Token>& token) { m_ownedTokens.push_back(std::move(const_cast<std::unique_ptr<Token>&>(token))); }
void Player::addPermanentResource(const ResourceType& resourceType, const uint8_t& quantity) { m_ownedPermanentResources[resourceType] += quantity; }
void Player::addTradingResource(const ResourceType& resourceType, const uint8_t& quantity) { m_ownedTradingResources[resourceType] += quantity; }
void Player::setRemainingCoins(const std::tuple<uint8_t,uint8_t,uint8_t>& remainingCoins) { m_remainingCoins = remainingCoins; }
void Player::setPoints(const Points& playerPoints) { m_playerPoints = playerPoints; }
void Player::addScientificSymbol(const ScientificSymbolType& symbol, const uint8_t& quantity) { m_ownedScientificSymbols[symbol] += quantity; }
const uint8_t Player::getkPlayerId() const { return kplayerId; }
const std::string& Player::getPlayerUsername() const { return m_playerUsername; }
std::vector<std::unique_ptr<Models::Wonder>>& Models::Player::getOwnedWonders() {return m_ownedWonders;}
const std::vector<std::unique_ptr<Token>>& Player::getOwnedTokens() const { return m_ownedTokens; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedPermanentResources() const { return m_ownedPermanentResources; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedTradingResources() const { return m_ownedTradingResources; }
const std::tuple<uint8_t,uint8_t,uint8_t> Player::getRemainingCoins() const { return m_remainingCoins; }
const std::unordered_map<ScientificSymbolType, uint8_t>& Player::getOwnedScientificSymbols() const { return m_ownedScientificSymbols; }
const Player::Points& Player::getPoints() const { return m_playerPoints; }
const std::unordered_map<TradeRuleType, bool>& Player::getTradeRules() const { return m_tradeRules; }

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
