module Models.AgeCard;
import Models.Card;
import Models.ScientificSymbolType;
import Models.AgeCard;
import Models.ResourceType;
import Models.LinkingSymbolType;
import Models.TradeRuleType;
import Models.Age;
import Models.CoinWorthType;
import Models.ColorType;
import <ostream>;
import <iostream>;
import <optional>;
import <unordered_map>;

using namespace Models;


const std::unordered_map<ResourceType, uint8_t>& AgeCard::getResourcesProduction() const { return m_resourceProduction; }
const uint8_t& AgeCard::getShieldPoints() const { return m_shieldPoints; }
const std::optional<ScientificSymbolType>& AgeCard::getScientificSymbols() const { return m_scientificSymbols; }
const std::optional<LinkingSymbolType>& AgeCard::getHasLinkingSymbol() const { return m_hasLinkingSymbol; }
const std::optional<LinkingSymbolType>& AgeCard::getRequiresLinkingSymbol() const { return m_requiresLinkingSymbol; }
const std::unordered_map<TradeRuleType, bool>& AgeCard::getTradeRules() const { return m_tradeRules; }
const Age& AgeCard::getAge() const { return m_age; }

const ResourceType& AgeCard::getResourceProduction() const {
	static const ResourceType kNone = ResourceType::NO_RESOURCE;
	return kNone;
}
bool AgeCard::IsConstructed() const { return false; }

// Setters
void AgeCard::setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
void AgeCard::setShieldPoints(const uint8_t& shieldPoints) { m_shieldPoints = shieldPoints; }
void AgeCard::setScientificSymbols(const std::optional<ScientificSymbolType>& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
void AgeCard::setHasLinkingSymbol(const std::optional<LinkingSymbolType>& hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
void AgeCard::setRequiresLinkingSymbol(const std::optional<LinkingSymbolType>& requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
void AgeCard::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_tradeRules = tradeRules; }
void AgeCard::setAge(const Age& age) { m_age = age; }

void AgeCard::displayCardInfo() {
	std::cout << "\n";
	Card::displayCardInfo();

	bool firstField = true;
	auto sep = [&]() { if (!firstField) std::cout << " | "; firstField = false; };

	if (!m_resourceProduction.empty()) {
		sep();
		std::cout << "Prod=";
		bool first = true;
		auto abbrev = [&](ResourceType r) {
			switch (r) {
			case ResourceType::WOOD: return 'W';
			case ResourceType::STONE: return 'S';
			case ResourceType::CLAY: return 'C';
			case ResourceType::PAPYRUS: return 'P';
			case ResourceType::GLASS: return 'G';
			default: return '?';
			}
		};
		for (const auto& kv : m_resourceProduction) {
			if (!first) std::cout << ','; first = false;
			std::cout << abbrev(kv.first) << ':' << static_cast<int>(kv.second);
		}
	}

	if (m_shieldPoints > 0) { sep(); std::cout << "SP=" << static_cast<int>(m_shieldPoints); }

	if (m_scientificSymbols.has_value()) { sep(); std::cout << "Sci=" << ScientificSymbolTypeToString(m_scientificSymbols.value()); }

	if (m_hasLinkingSymbol.has_value()) { sep(); std::cout << "HasLS=" << LinkingSymbolTypeToString(m_hasLinkingSymbol.value()); }
	if (m_requiresLinkingSymbol.has_value()) { sep(); std::cout << "ReqLS=" << LinkingSymbolTypeToString(m_requiresLinkingSymbol.value()); }

	if (!m_tradeRules.empty()) {
		bool any = false;
		std::string tr;
		for (const auto& kv : m_tradeRules) {
			if (kv.second) {
				if (any) tr.push_back(',');
				tr += ResourceTypeToString(static_cast<ResourceType>(kv.first));
				any = true;
			}
		}
		if (any) { sep(); std::cout << "TR=" << tr; }
	}

	std::cout << "\n";
}

std::ostream& Models::operator<<(std::ostream& os, const AgeCard& card)
{
	os << "Card Name: " << card.getName() << '\n';
	os << "Color: " << ColorTypeToString(card.getColor()) << '\n';
	os << "Caption: " << card.getCaption() << '\n';
	os << "Resource Cost:" << '\n';
	for (const auto& [res, amt] : card.getResourceCost())
		os << " - " << ResourceTypeToString(res) << ": " << static_cast<int>(amt) << '\n';
	os << "Resource Production:" << '\n';
	for (const auto& [res, amt] : card.getResourcesProduction())
		os << " - " << ResourceTypeToString(res) << ": " << static_cast<int>(amt) << '\n';
	os << "Victory Points: " << static_cast<int>(card.getVictoryPoints()) << '\n';
	os << "Shield Points: " << static_cast<int>(card.getShieldPoints()) << '\n';
	if (card.getScientificSymbols().has_value()) os << "Scientific Symbol: " << ScientificSymbolTypeToString(card.getScientificSymbols().value()) << '\n';
	if (card.getHasLinkingSymbol().has_value()) os << "Linking Symbol Provided: " << LinkingSymbolTypeToString(card.getHasLinkingSymbol().value()) << '\n';
	if (card.getRequiresLinkingSymbol().has_value()) os << "Linking Symbol Required: " << LinkingSymbolTypeToString(card.getRequiresLinkingSymbol().value()) << '\n';
	os << "Trade Rules:" << '\n';
	for (const auto& [rule, enabled] : card.getTradeRules())
		os << " - " << ResourceTypeToString(static_cast<ResourceType>(rule)) << ": " << (enabled ? "Enabled" : "Disabled") << '\n';
	os << "Age: " << static_cast<int>(card.getAge()) << '\n';
	return os;
}

AgeCardBuilder& AgeCardBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
AgeCardBuilder& AgeCardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
AgeCardBuilder& AgeCardBuilder::setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) { m_card.setResourceProduction(resourceProduction); return *this; }
AgeCardBuilder& AgeCardBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
AgeCardBuilder& AgeCardBuilder::setShieldPoints(const uint8_t& shieldPoints) { m_card.setShieldPoints(shieldPoints); return *this; }
AgeCardBuilder& AgeCardBuilder::setScientificSymbols(const std::optional<ScientificSymbolType>& scientificSymbols) { m_card.setScientificSymbols(scientificSymbols); return *this; }
AgeCardBuilder& AgeCardBuilder::setHasLinkingSymbol(const std::optional<LinkingSymbolType>& hasLinkingSymbol) { m_card.setHasLinkingSymbol(hasLinkingSymbol); return *this; }
AgeCardBuilder& AgeCardBuilder::setRequiresLinkingSymbol(const std::optional<LinkingSymbolType>& requiresLinkingSymbol) { m_card.setRequiresLinkingSymbol(requiresLinkingSymbol); return *this; }
AgeCardBuilder& AgeCardBuilder::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_card.setTradeRules(tradeRules); return *this; }
AgeCardBuilder& AgeCardBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
AgeCardBuilder& AgeCardBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
AgeCardBuilder& AgeCardBuilder::setAge(const Age& age) { m_card.setAge(age); return *this; }
AgeCardBuilder& AgeCardBuilder::addOnPlayAction(const std::function<void()>& action) { m_card.addOnPlayAction(action); return *this; }
AgeCardBuilder& AgeCardBuilder::addOnDiscardAction(const std::function<void()>& action) { m_card.addOnDiscardAction(action); return *this; }
AgeCard AgeCardBuilder::build() { return std::move(m_card); }

void AgeCard::onDiscard() { Card::onDiscard(); }

