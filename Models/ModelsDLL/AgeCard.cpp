module Models.AgeCard;
import Models.Card;
import Models.ScientificSymbolType;
import Models.ResourceType;
import Models.LinkingSymbolType;
import Models.TradeRuleType;
import Models.Age;
import Models.CoinWorthType;
import Models.ColorType;
import <ostream>;
import <iostream>;
import <unordered_map>;

using namespace Models;



const std::unordered_map<ResourceType, uint8_t>& AgeCard::getResourceProduction() const { return m_resourceProduction; }
uint8_t AgeCard::getShieldPoints() const { return m_shieldPoints; }
ScientificSymbolType AgeCard::getScientificSymbols() const { return m_scientificSymbols; }
LinkingSymbolType AgeCard::getHasLinkingSymbol() const { return m_hasLinkingSymbol; }
LinkingSymbolType AgeCard::getRequiresLinkingSymbol() const { return m_requiresLinkingSymbol; }
const std::unordered_map<TradeRuleType, bool>& AgeCard::getTradeRules() const { return m_tradeRules; }
Age AgeCard::getAge() const { return m_age; }
void AgeCard::setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
void AgeCard::setShieldPoints(uint8_t shieldPoints) { m_shieldPoints = shieldPoints; }
void AgeCard::setScientificSymbols(ScientificSymbolType scientificSymbols) { m_scientificSymbols = scientificSymbols; }
void AgeCard::setHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
void AgeCard::setRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
void AgeCard::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_tradeRules = tradeRules; }
void AgeCard::setAge(const Age& age) { m_age = age; }

AgeCard::AgeCard(
	const std::string& name,
	const std::unordered_map<ResourceType, uint8_t>& resourceCost,
	const std::unordered_map<ResourceType, uint8_t>& resourceProduction,
	uint8_t victoryPoints,
	uint8_t shieldPoints,
	uint8_t coinCost,
	ScientificSymbolType scientificSymbols,
	LinkingSymbolType hasLinkingSymbol,
	LinkingSymbolType requiresLinkingSymbol,
	CoinWorthType coinWorth,
	uint8_t coinReward,
	const std::unordered_map<TradeRuleType, bool>& tradeRules,
	const std::string& caption,
	ColorType color,
	bool isVisible,
	const std::string& modelPath,
	Age age)
	: Card(name, resourceCost, victoryPoints, coinWorth, coinReward, caption, color, isVisible, modelPath)
{
	m_resourceProduction = resourceProduction;
	m_shieldPoints = shieldPoints;
	m_scientificSymbols = scientificSymbols;
	m_hasLinkingSymbol = hasLinkingSymbol;
	m_requiresLinkingSymbol = requiresLinkingSymbol;
	m_tradeRules = tradeRules;
	m_age = age;
}

void AgeCard::toggleVisibility() { Card::toggleVisibility(); }
void AgeCard::toggleAccessibility() { Card::toggleAccessibility(); }
void AgeCard::displayCardInfo() {
	Card::displayCardInfo();
	std::cout << " Resource Production: ";
	for (const auto& kv : m_resourceProduction) {
		std::cout << ResourceTypeToString(kv.first) << ":" << static_cast<int>(kv.second) << " ";
	}
	std::cout << "\n";
	std::cout << " Shield Points: " << static_cast<int>(m_shieldPoints) << "\n";
	std::cout << " Scientific Symbol: " << ScientificSymbolTypeToString(m_scientificSymbols) << "\n";
	std::cout << " Has Linking Symbol: " << LinkingSymbolTypeToString(m_hasLinkingSymbol) << "\n";
	std::cout << " Requires Linking Symbol: " << LinkingSymbolTypeToString(m_requiresLinkingSymbol) << "\n";
	std::cout << " Trade Rules: ";
	for (const auto& kv : m_tradeRules) {
		std::cout << ResourceTypeToString(static_cast<ResourceType>(kv.first)) << ":" << (kv.second ? "true" : "false") << " ";
	}
	std::cout << "\n";
	std::cout << " Age: " << static_cast<int>(m_age) << "\n";
}

std::ostream& Models::operator<<(std::ostream& os, const AgeCard& card)
{
	os << "Card Name: " << card.GetName() << '\n';
	os << "Color: " << ColorTypeToString(card.GetColor()) << '\n';
	os << "Caption: " << card.GetCaption() << '\n';
	os << "Model Path: " << card.GetModelPath() << '\n';
	os << "Visible: " << (card.GetIsVisible() ? "Yes" : "No") << '\n';
	os << "Resource Cost:" << '\n';
	for (const auto& [res, amt] : card.GetResourceCost())
		os << " - " << ResourceTypeToString(res) << ": " << static_cast<int>(amt) << '\n';
	os << "Resource Production:" << '\n';
	for (const auto& [res, amt] : card.getResourceProduction())
		os << " - " << ResourceTypeToString(res) << ": " << static_cast<int>(amt) << '\n';
	os << "Victory Points: " << static_cast<int>(card.GetVictoryPoints()) << '\n';
	os << "Shield Points: " << static_cast<int>(card.getShieldPoints()) << '\n';
	os << "Scientific Symbol: " << ScientificSymbolTypeToString(card.getScientificSymbols()) << '\n';
	os << "Linking Symbol Provided: " << LinkingSymbolTypeToString(card.getHasLinkingSymbol()) << '\n';
	os << "Linking Symbol Required: " << LinkingSymbolTypeToString(card.getRequiresLinkingSymbol()) << '\n';
	os << "Coin Worth Type: " << CoinWorthTypeToString(card.GetCoinWorth()) << '\n';
	os << "Coin Reward: " << static_cast<int>(card.GetCoinReward()) << '\n';
	os << "Trade Rules:" << '\n';
	for (const auto& [rule, enabled] : card.getTradeRules())
		os << " - " << ResourceTypeToString(static_cast<ResourceType>(rule)) << ": " << (enabled ? "Enabled" : "Disabled") << '\n';
	os << "Age: " << static_cast<int>(card.getAge()) << '\n';
	return os;
}

