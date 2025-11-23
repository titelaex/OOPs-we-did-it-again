module Models.Wonder;
import Models.Card;
import Models.Player; // ensure Player available for action methods
import Models.ResourceType;
import Models.Token;
import <random>;
import <algorithm>;
import <iterator>;
import <ostream>;
import <iostream>;
import <unordered_map>;

using namespace Models;

Wonder::Wonder(Wonder&& other) noexcept = default;
Wonder& Wonder::operator=(Wonder&& other) noexcept = default;

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