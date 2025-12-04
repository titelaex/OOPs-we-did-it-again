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
const uint8_t& AgeCard::getShieldPoints() const { return m_shieldPoints; }
const ScientificSymbolType& AgeCard::getScientificSymbols() const { return m_scientificSymbols; }
const LinkingSymbolType& AgeCard::getHasLinkingSymbol() const { return m_hasLinkingSymbol; }
const LinkingSymbolType& AgeCard::getRequiresLinkingSymbol() const { return m_requiresLinkingSymbol; }
const std::unordered_map<TradeRuleType, bool>& AgeCard::getTradeRules() const { return m_tradeRules; }
const Age& AgeCard::getAge() const { return m_age; }
void AgeCard::setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
void AgeCard::setShieldPoints(const uint8_t& shieldPoints) { m_shieldPoints = shieldPoints; }
void AgeCard::setScientificSymbols(const ScientificSymbolType& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
void AgeCard::setHasLinkingSymbol(const LinkingSymbolType& hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
void AgeCard::setRequiresLinkingSymbol(const LinkingSymbolType& requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
void AgeCard::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_tradeRules = tradeRules; }
void AgeCard::setAge(const Age& age) { m_age = age; }

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

AgeCardBuilder& AgeCardBuilder::setName(const std::string& name) {
    m_card.setName(name);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) {
    m_card.setResourceCost(resourceCost);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) {
    m_card.setResourceProduction(resourceProduction);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setVictoryPoints(const uint8_t& victoryPoints) {
    m_card.setVictoryPoints(victoryPoints);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setShieldPoints(const uint8_t& shieldPoints) {
    m_card.setShieldPoints(shieldPoints);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setCoinWorth(const CoinWorthType& coinWorth) {
    m_card.setCoinWorth(coinWorth);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setCoinReward(const uint8_t& coinReward) {
    m_card.setCoinReward(coinReward);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setScientificSymbols(const ScientificSymbolType& scientificSymbols) {
    m_card.setScientificSymbols(scientificSymbols);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setHasLinkingSymbol(const LinkingSymbolType& hasLinkingSymbol) {
    m_card.setHasLinkingSymbol(hasLinkingSymbol);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setRequiresLinkingSymbol(const LinkingSymbolType& requiresLinkingSymbol) {
    m_card.setRequiresLinkingSymbol(requiresLinkingSymbol);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) {
    m_card.setTradeRules(tradeRules);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setCaption(const std::string& caption) {
    m_card.setCaption(caption);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setColor(const ColorType& color) {
    m_card.setColor(color);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::setAge(const Age& age) {
    m_card.setAge(age);
    return *this;
}

AgeCardBuilder& AgeCardBuilder::addOnPlayAction(const std::function<void()>& action) {
    m_card.addOnPlayAction(action);
    return *this;
}

AgeCard AgeCardBuilder::build() {
    return std::move(m_card);
}

void AgeCard::onDiscard()
{
    Card::onDiscard();
}

