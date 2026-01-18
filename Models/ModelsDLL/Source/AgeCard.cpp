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
import <string>;
import <utility>;

using namespace Models;


const std::unordered_map<ResourceType, uint8_t>& AgeCard::getResourcesProduction() const { return m_resourceProduction; }
const uint8_t& AgeCard::getShieldPoints() const { return m_shieldPoints; }
uint8_t AgeCard::getCoinCost() const { return m_coinCost; }
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
void AgeCard::setCoinCost(uint8_t coinCost) { m_coinCost = coinCost; }
void AgeCard::setScientificSymbols(const std::optional<ScientificSymbolType>& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
void AgeCard::setHasLinkingSymbol(const std::optional<LinkingSymbolType>& hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
void AgeCard::setRequiresLinkingSymbol(const std::optional<LinkingSymbolType>& requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
void AgeCard::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_tradeRules = tradeRules; }
void AgeCard::setAge(const Age& age) { m_age = age; }



namespace
{
	std::string csvEscape(const std::string& s) {
		if (s.empty()) return "";
		std::string out;
		// No need to check for find_first_of, always quote for consistency as requested
		out.reserve(s.size() + 2);
		out.push_back('"');
		for (char ch : s) {
			if (ch == '"') out += "\"\"";
			else out.push_back(ch);
		}
		out.push_back('"');
		return out;
	}

	std::string resourceMapToString(const std::unordered_map<ResourceType, uint8_t>& map) {
		std::string s;
		bool first = true;
		for (const auto& kv : map) {
			if (!first) s += ",";
			first = false;
			s += ResourceTypeToString(kv.first);
			s.push_back(':');
			s += std::to_string(static_cast<int>(kv.second));
		}
		return s;
	}

	std::string tradeRuleMapToString(const std::unordered_map<TradeRuleType, bool>& map) {
		std::string s;
		bool first = true;
		for (const auto& kv : map) {
			if (!kv.second) continue;
			if (!first) s += ";";
			first = false;
			s += tradeRuleTypeToString(kv.first);
			s += ":true";
		}
		return s;
	}

	std::string actionPairVectorToString(const std::vector<std::pair<std::function<void()>, std::string>>& actions) {
		std::string s;
		bool first = true;
		for (const auto& actionPair : actions) {
			if (!first) s += ",";
			first = false;
			s += actionPair.second;
		}
		return s;
	}

	std::string ageToCsvString(const Age& age) {
		switch (age) {
		case Age::AGE_I: return "AGE_I";
		case Age::AGE_II: return "AGE_II";
		case Age::AGE_III: return "AGE_III";
		default: return "";
		}
	}

	void writeEscapedField(std::ostream& out, const std::string& value) {
		if (!value.empty()) out << csvEscape(value);
		out << ',';
	}

	void writeNumericField(std::ostream& out, uint8_t value) {
		if (value > 0) out << static_cast<int>(value);
		out << ',';
	}

	void writeFinalEscapedField(std::ostream& out, const std::string& value) {
		if (!value.empty()) out << csvEscape(value);
	}
}

__declspec(dllexport) std::ostream& Models::operator<<(std::ostream& out, const AgeCard& card)
{
	writeEscapedField(out, card.getName());
	writeEscapedField(out, resourceMapToString(card.getResourceCost()));
	writeEscapedField(out, resourceMapToString(card.getResourcesProduction()));
	writeNumericField(out, card.getVictoryPoints());
	writeNumericField(out, card.getShieldPoints());
	writeNumericField(out, card.getCoinCost()); // write coin cost

	std::string scientificSymbol = card.getScientificSymbols().has_value() ? ScientificSymbolTypeToString(card.getScientificSymbols().value()) : std::string{};
	writeEscapedField(out, scientificSymbol);

	std::string hasLink = card.getHasLinkingSymbol().has_value() ? LinkingSymbolTypeToString(card.getHasLinkingSymbol().value()) : std::string{};
	writeEscapedField(out, hasLink);

	std::string requiresLink = card.getRequiresLinkingSymbol().has_value() ? LinkingSymbolTypeToString(card.getRequiresLinkingSymbol().value()) : std::string{};
	writeEscapedField(out, requiresLink);

	writeEscapedField(out, tradeRuleMapToString(card.getTradeRules()));
	writeEscapedField(out, card.getCaption());
	writeEscapedField(out, ColorTypeToString(card.getColor()));
	writeEscapedField(out, ageToCsvString(card.getAge()));
	writeEscapedField(out, actionPairVectorToString(card.getOnPlayActions()));
	writeFinalEscapedField(out, actionPairVectorToString(card.getOnDiscardActions()));

	return out;
}

AgeCardBuilder& AgeCardBuilder::setName(const std::string& name) { m_card->setName(name); return *this; }
AgeCardBuilder& AgeCardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card->setResourceCost(resourceCost); return *this; }
AgeCardBuilder& AgeCardBuilder::setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) { m_card->setResourceProduction(resourceProduction); return *this; }
AgeCardBuilder& AgeCardBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card->setVictoryPoints(victoryPoints); return *this; }
AgeCardBuilder& AgeCardBuilder::setShieldPoints(const uint8_t& shieldPoints) { m_card->setShieldPoints(shieldPoints); return *this; }
AgeCardBuilder& AgeCardBuilder::setCoinCost(uint8_t coinCost) { m_card->setCoinCost(coinCost); return *this; }
AgeCardBuilder& AgeCardBuilder::setScientificSymbols(const std::optional<ScientificSymbolType>& scientificSymbols) { m_card->setScientificSymbols(scientificSymbols); return *this; }
AgeCardBuilder& AgeCardBuilder::setHasLinkingSymbol(const std::optional<LinkingSymbolType>& hasLinkingSymbol) { m_card->setHasLinkingSymbol(hasLinkingSymbol); return *this; }
AgeCardBuilder& AgeCardBuilder::setRequiresLinkingSymbol(const std::optional<LinkingSymbolType>& requiresLinkingSymbol) { m_card->setRequiresLinkingSymbol(requiresLinkingSymbol); return *this; }
AgeCardBuilder& AgeCardBuilder::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_card->setTradeRules(tradeRules); return *this; }
AgeCardBuilder& AgeCardBuilder::setCaption(const std::string& caption) { m_card->setCaption(caption); return *this; }
AgeCardBuilder& AgeCardBuilder::setColor(const ColorType& color) { m_card->setColor(color); return *this; }
AgeCardBuilder& AgeCardBuilder::setAge(const Age& age) { m_card->setAge(age); return *this; }
AgeCardBuilder& AgeCardBuilder::addOnPlayAction(const std::function<void()>& action, std::string actionString) { m_card->addOnPlayAction(action, std::move(actionString)); return *this; }
AgeCardBuilder& AgeCardBuilder::addOnDiscardAction(const std::function<void()>& action, std::string actionString) { m_card->addOnDiscardAction(action, std::move(actionString)); return *this; }
AgeCard AgeCardBuilder::build() { return std::move(*m_card); }

void AgeCard::onDiscard() { Card::onDiscard(); }

