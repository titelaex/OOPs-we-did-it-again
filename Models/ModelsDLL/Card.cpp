module Models.Card;
import Models.Card;
import Models.Age;
import <iostream>;

using namespace Models;

Card::Card(Card&& other) = default;
Card& Card::operator=(Card&& other) = default;

const std::string& Card::GetName() const { return m_name; }
const std::unordered_map<ResourceType, uint8_t>& Card::GetResourceCost() const { return m_resourceCost; }
uint8_t Card::GetVictoryPoints() const { return m_victoryPoints; }
CoinWorthType Card::GetCoinWorth() const { return m_coinWorth; }
uint8_t Card::GetCoinReward() const { return m_coinReward; }
const std::string& Card::GetCaption() const { return m_caption; }
ColorType Card::GetColor() const { return m_color; }
bool Card::GetIsVisible() const { return m_isVisibile; }
bool Card::GetIsAccessible() const { return m_isAccessible; }
const std::string& Card::GetModelPath() const { return m_modelPath; }

void Card::SetName(const std::string& name) { m_name = name; }
void Card::SetResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
void Card::SetVictoryPoints(uint8_t victoryPoints) { m_victoryPoints = victoryPoints; }
void Card::SetCoinReward(uint8_t coinReward) { m_coinReward = coinReward; }
void Card::SetCoinWorth(CoinWorthType coinWorth) { m_coinWorth = coinWorth; }
void Card::SetCaption(const std::string& caption) { m_caption = caption; }
void Card::SetColor(ColorType color) { m_color = color; }
void Card::SetIsVisible(bool isVisible) { m_isVisibile = isVisible; }
void Card::SetIsAccessible(bool isAccessible) { m_isAccessible = isAccessible; }
void Card::SetModelPath(const std::string& modelPath) { m_modelPath = modelPath; }

void Card::toggleVisibility() { m_isVisibile = !m_isVisibile; }
void Card::toggleAccessibility() { m_isAccessible = !m_isAccessible; }
void Card::displayCardInfo()
{

}

Card::Card(
	const std::string& name,
	const std::unordered_map<ResourceType, uint8_t>& resourceCost,
	uint8_t victoryPoints,
	CoinWorthType coinWorth,
	uint8_t coinReward,
	const std::string& caption,
	ColorType color,
	bool isVisible,
	const std::string& modelPath)
{
	m_name = name;
	m_resourceCost = resourceCost;
	m_victoryPoints = victoryPoints;
	m_coinWorth = coinWorth;
	m_coinReward = coinReward;
	m_caption = caption;
	m_color = color;
	m_isVisibile = isVisible;
	m_modelPath = modelPath;
}
