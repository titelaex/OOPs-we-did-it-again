module Models.Wonder;
import Models.Card;
import Models.ResourceType;
import Models.Token;
import <random>;
import <algorithm>;
import <iterator>;
import <ostream>;
import <iostream>;
import <unordered_map>;
import <bitset>;

using namespace Models;

Wonder::Wonder(
	const std::string& name,
	const std::unordered_map<ResourceType, uint8_t>& resourceCost,
	uint8_t victoryPoints,
	CoinWorthType coinWorth,
	uint8_t coinReward,
	const std::string& caption,
	ColorType color,
	bool isVisible,
	const std::string& modelPath,

	uint8_t opponentLosesMoney,
	uint8_t shieldPoints,
	ResourceType resourceProduction,
	bool playSecondTurn,
	bool drawProgressTokens,
	bool discardCardFromOpponent,
	bool chooseAndConstructBuilding,
	ColorType discardedCardColor)
	: Card(name, resourceCost, victoryPoints, coinWorth, coinReward, caption, color, isVisible, modelPath)
{
	m_opponentLosesMoney = opponentLosesMoney;
	m_shieldPoints = shieldPoints;
	m_resourceProduction = resourceProduction;
	m_flags.reset();
	m_flags.set(0, playSecondTurn);
	m_flags.set(1, drawProgressTokens);
	m_flags.set(2, chooseAndConstructBuilding);
	m_flags.set(3, discardCardFromOpponent);
	m_flags.set(4, false); // is constructed
	m_discardedCardColor = discardedCardColor;
}

uint8_t Wonder::getOpponentLosesMoney() const { return m_opponentLosesMoney; }
uint8_t Wonder::getShieldPoints() const { return m_shieldPoints; }
ResourceType Wonder::getResourceProduction() const { return m_resourceProduction; }

bool Wonder::getPlaySecondTurn() const { return m_flags.test(0); }
bool Wonder::getDrawProgressTokens() const { return m_flags.test(1); }
bool Wonder::getChooseAndConstructBuilding() const { return m_flags.test(2); }
bool Wonder::getDiscardCardFromOpponent() const { return m_flags.test(3); }
bool Wonder::getIsConstructed() const { return m_flags.test(4); }

void Wonder::setOpponentLosesMoney(uint8_t amt) { m_opponentLosesMoney = amt; }
void Wonder::setShieldPoints(uint8_t pts) { m_shieldPoints = pts; }
void Wonder::setResourceProduction(ResourceType r) { m_resourceProduction = r; }

void Wonder::setPlaySecondTurn(bool v) { m_flags.set(0, v); }
void Wonder::setDrawProgressTokens(bool v) { m_flags.set(1, v); }
void Wonder::setChooseAndConstructBuilding(bool v) { m_flags.set(2, v); }
void Wonder::setDiscardCardFromOpponent(bool v) { m_flags.set(3, v); }
void Wonder::setIsConstructed(bool v) { m_flags.set(4, v); }

std::ostream& Models::operator<<(std::ostream& os, const Wonder& card)
{
	os << "Card Name: " << card.GetName() << '\n';
	os << "Color: " << static_cast<int>(card.GetColor()) << '\n';
	os << "Caption: " << card.GetCaption() << '\n';
	os << "Model Path: " << card.GetModelPath() << '\n';
	os << "Visible: " << (card.GetIsVisible() ? "Yes" : "No") << '\n';
	os << "Resource Cost:" << '\n';
	for (const auto& [res, amt] : card.GetResourceCost())
		os << " - " << static_cast<int>(res) << ": " << static_cast<int>(amt) << '\n';
	os << "Victory Points: " << static_cast<int>(card.GetVictoryPoints()) << '\n';
	os << "Coin Worth Type: " << static_cast<int>(card.GetCoinWorth()) << '\n';
	os << "Coin Reward: " << static_cast<int>(card.GetCoinReward()) << '\n';
	os << "Opponent Loses Money: " << static_cast<int>(card.getOpponentLosesMoney()) << '\n';
	os << "Play Second Turn: " << (card.getPlaySecondTurn() ? "Yes" : "No") << '\n';
	os << "Draw Progress Tokens: " << (card.getDrawProgressTokens() ? "Yes" : "No") << '\n';
	os << "Choose and Construct Building: " << (card.getChooseAndConstructBuilding() ? "Yes" : "No") << '\n';
	os << "Discard Card From Opponent: " << (card.getDiscardCardFromOpponent() ? "Yes" : "No") << '\n';
	return os;
}

void Wonder::displayCardInfo()
{
	Card::displayCardInfo();
	std::cout << " Resource Production: " << ResourceTypeToString(m_resourceProduction) << "\n";
	std::cout << " Shield Points: " << static_cast<int>(m_shieldPoints) << "\n";
	std::cout << " Opponent Loses Money: " << static_cast<int>(m_opponentLosesMoney) << "\n";
	std::cout << " Play Second Turn: " << (getPlaySecondTurn() ? "Yes" : "No") << "\n";
	std::cout << " Draw Progress Tokens: " << (getDrawProgressTokens() ? "Yes" : "No") << "\n";
	std::cout << " Choose and Construct Building: " << (getChooseAndConstructBuilding() ? "Yes" : "No") << "\n";
	std::cout << " Discard Card From Opponent: " << (getDiscardCardFromOpponent() ? "Yes" : "No") << "\n";
	std::cout << " Discarded Card Color: " << static_cast<int>(m_discardedCardColor) << "\n";
}