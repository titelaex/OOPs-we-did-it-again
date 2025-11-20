module Models.Card;
import Models.Card;
import Models.Age;
import <iostream>;

using namespace Models;

Card::Card(Card&& other) noexcept = default;
Card& Card::operator=(Card&& other) noexcept = default;

const std::string& Card::GetName() const { return m_name; }
const std::unordered_map<ResourceType, uint8_t>& Card::GetResourceCost() const { return m_resourceCost; }
const std::unordered_map<ResourceType, uint8_t>& Card::GetResourceProduction() const { return m_resourceProduction; }
uint8_t Card::GetVictoryPoints() const { return m_victoryPoints; }
uint8_t Card::GetShieldPoints() const { return m_shieldPoints; }
uint8_t Card::GetCoinCost() const { return coinCost; }
const std::array<uint8_t, 7>& Card::GetScientificSymbols() const { return m_scientificSymbols; }
LinkingSymbolType Card::GetHasLinkingSymbol() const { return m_hasLinkingSymbol; }
LinkingSymbolType Card::GetRequiresLinkingSymbol() const { return m_requiresLinkingSymbol; }
CoinWorthType Card::GetCoinWorth() const { return m_coinWorth; }
uint8_t Card::GetCoinValue() const { return m_coinValue; }
const std::unordered_map<TradeRuleType, bool>& Card::GetTradeRules() const { return m_tradeRules; }
const std::string& Card::GetCaption() const { return m_caption; }
ColorType Card::GetColor() const { return m_color; }
bool Card::GetIsVisible() const { return m_isVisibile; }
const std::string& Card::GetModelPath() const { return m_modelPath; }

void Card::SetName(const std::string& name) { m_name = name; }
void Card::SetResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
void Card::SetResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
void Card::SetVictoryPoints(uint8_t victoryPoints) { m_victoryPoints = victoryPoints; }
void Card::SetShieldPoints(uint8_t shieldPoints) { m_shieldPoints = shieldPoints; }
void Card::SetCoinCost(uint8_t cost) { coinCost = cost; }
void Card::SetScientificSymbols(const std::array<uint8_t, 7>& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
void Card::SetHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
void Card::SetRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
void Card::SetCoinValue(uint8_t coinValue) { m_coinValue = coinValue; }
void Card::SetTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_tradeRules = tradeRules; }
void Card::SetCoinWorth(CoinWorthType coinWorth) { m_coinWorth = coinWorth; }
void Card::SetCaption(const std::string& caption) { m_caption = caption; }
void Card::SetColor(ColorType color) { m_color = color; }
void Card::SetIsVisible(bool isVisible) { m_isVisibile = isVisible; }
void Card::SetModelPath(const std::string& modelPath) { m_modelPath = modelPath; }
void Card::SetAge(const Age& age) { m_age = age; }

void Card::toggleVisibility() { m_isVisibile = !m_isVisibile; }
void Card::toggleAccessibility() { m_isAccessible = !m_isAccessible; }
void Card::displayCardInfo()
{
	std::cout << "Card: " << m_name << " (Age " << static_cast<int>(m_age) << ")\n";
}
