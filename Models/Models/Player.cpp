module Models.Player;
import Models.Player;
import <iostream>;

using namespace Models;

uint8_t Player::totalCoins(std::tuple<uint8_t, uint8_t, uint8_t> coins) { return std::get<0>(coins) + std::get<1>(coins) * 3 + std::get<2>(coins) * 6; }

uint8_t Player::Points::totalVictoryPoints() { return m_militaryVictoryPoints + m_buildingVictoryPoints + m_wonderVictoryPoints + progressVictoryPoints; }

Player::Player(const uint8_t& id, const std::string& username) : kplayerId(id), kplayerUsername(username)
{
	m_playerPoints = { 0,0,0,0 };
}


void Player::addCard(const Card& card) { m_ownedCards.push_back(card); }
void Player::addWonder(const Wonder& wonder) { m_ownedWonders.push_back(wonder); }
void Player::addToken(const Token& token) { m_ownedTokens.push_back(token); }
void Player::addPermanentResource(ResourceType resourceType, uint8_t quantity) { m_ownedPermanentResources[resourceType] += quantity; }
void Player::addTradingResource(ResourceType resourceType, uint8_t quantity) { m_ownedTradingResources[resourceType] += quantity; }
void Player::setHasConflictPawn(bool hasPawn) { m_hasConflictPawn = hasPawn; }
void Player::setRemainingCoins(std::tuple<uint8_t,uint8_t,uint8_t> remainingCoins) { m_remainingCoins = remainingCoins; }
void Player::setPoints(Points playerPoints) { m_playerPoints = playerPoints; }
const uint8_t Player::getkPlayerId() const { return kplayerId; }
const std::string& Player::getPlayerUsername() const { return kplayerUsername; }
const std::vector<Wonder>& Player::getOwnedWonders() const { return m_ownedWonders; }
const std::vector<Card>& Player::getOwnedCards() const { return m_ownedCards; }
const std::vector<Token>& Player::getOwnedTokens() const { return m_ownedTokens; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedPermanentResources() const { return m_ownedPermanentResources; }
const std::unordered_map<ResourceType,uint8_t>& Player::getOwnedTradingResources() const { return m_ownedTradingResources; }
bool Player::getHasConflictPawn() const { return m_hasConflictPawn; }
std::tuple<uint8_t,uint8_t,uint8_t> Player::getRemainingCoins() const { return m_remainingCoins; }
Player::Points Player::getPoints() const { return m_playerPoints; }

void Player::addCoins(uint8_t coinsToAdd)
{
	auto& [ones, threes, sixes] = m_remainingCoins;
	while (coinsToAdd >= 6) { ++sixes; coinsToAdd -= 6; }
	while (coinsToAdd >= 3) { ++threes; coinsToAdd -= 3; }
	while (coinsToAdd > 0) { ++ones; --coinsToAdd; }
}

void Player::subtractCoins(uint8_t coinsToSubtract)
{
	auto& [ones, threes, sixes] = m_remainingCoins;
	uint8_t total = totalCoins(m_remainingCoins);
	if (coinsToSubtract > total) { ones = threes = sixes = 0; return; }
	while (coinsToSubtract >= 6 && sixes > 0) { --sixes; coinsToSubtract -= 6; }
	while (coinsToSubtract >= 3 && threes > 0) { --threes; coinsToSubtract -= 3; }
	while (coinsToSubtract > 0 && ones > 0) { --ones; --coinsToSubtract; }
	while (coinsToSubtract > 0) {
		if (sixes > 0) { --sixes; addCoins(6 - coinsToSubtract); coinsToSubtract = 0; }
		else if (threes > 0) { --threes; addCoins(3 - coinsToSubtract); coinsToSubtract = 0; }
		else break;
	}
}

uint8_t Player::countYellowCards() const
{
	uint8_t count = 0;
	for (const auto& card : m_ownedCards)
		if (card.GetColor() == ColorType::YELLOW) ++count;
	return count;
}

void Player::burnCard(Card& card)
{
	uint8_t yellowCards = countYellowCards();
	uint8_t coinsEarned = 2 + yellowCards;
	addCoins(coinsEarned);
	card.SetIsVisible(false);
	std::cout << "Card \"" << card.GetName() << "\" discarded. Player \"" << getPlayerUsername() << "\" gains " << static_cast<int>(coinsEarned) << " coins.\n";
}
