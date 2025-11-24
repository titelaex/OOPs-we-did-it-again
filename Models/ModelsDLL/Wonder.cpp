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

	ResourceType resourceProduction,
	uint8_t shieldPoints,
	uint8_t playerReceivesMoney,
	uint8_t opponentLosesMoney,
	bool discardCardFromOpponent,
	bool playSecondTurn,
	bool drawProgressTokens,
	bool chooseAndConstructBuilding,
	ColorType discardedCardColor)
	: Card(name, resourceCost, victoryPoints, coinWorth, coinReward, caption, color, isVisible, modelPath)
{
	m_resourceProduction = resourceProduction;
	m_shieldPoints = shieldPoints;
	m_playerReceivesMoney = playerReceivesMoney;
	m_opponentLosesMoney = opponentLosesMoney;
	m_discardCardFromOpponent = discardCardFromOpponent;
	m_playSecondTurn = playSecondTurn;
	m_drawProgressTokens = drawProgressTokens;
	m_chooseAndConstructBuilding = chooseAndConstructBuilding;
	m_discardedCardColor = discardedCardColor;
}


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
	os << "Player Receives Money: " << static_cast<int>(card.getPlayerReceivesMoney()) << '\n';
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
	std::cout << " Resource Production: " << static_cast<int>(m_resourceProduction) << "\n";
	std::cout << " Shield Points: " << static_cast<int>(m_shieldPoints) << "\n";
	std::cout << " Player Receives Money: " << static_cast<int>(m_playerReceivesMoney) << "\n";
	std::cout << " Opponent Loses Money: " << static_cast<int>(m_opponentLosesMoney) << "\n";
	std::cout << " Play Second Turn: " << (m_playSecondTurn ? "Yes" : "No") << "\n";
	std::cout << " Draw Progress Tokens: " << (m_drawProgressTokens ? "Yes" : "No") << "\n";
	std::cout << " Choose and Construct Building: " << (m_chooseAndConstructBuilding ? "Yes" : "No") << "\n";
	std::cout << " Discard Card From Opponent: " << (m_discardCardFromOpponent ? "Yes" : "No") << "\n";
	std::cout << " Discarded Card Color: " << static_cast<int>(m_discardedCardColor) << "\n";
}