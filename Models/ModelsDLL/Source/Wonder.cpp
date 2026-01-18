module Models.Wonder;
import <cstdint>;
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
import <string>;
import <vector>;
import <functional>;
import <utility>;


using namespace Models;

namespace
{
	std::string csvEscape(const std::string& s) {
		if (s.empty()) return "";
		std::string out;
		out.push_back('"');
		for (char ch : s) {
			if (ch == '"') out += "\"\"";
			else out.push_back(ch);
		}
		out.push_back('"');
		return out;
	}

	std::string resourceMapToString(const std::unordered_map<Models::ResourceType, uint8_t>& map) {
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

	void writeEscapedField(std::ostream& os, const std::string& value) {
		if (!value.empty()) os << csvEscape(value);
		os << ',';
	}

	void writeNumericField(std::ostream& os, uint8_t value) {
		if (value > 0) os << static_cast<int>(value);
		os << ',';
	}

	void writeFinalEscapedField(std::ostream& os, const std::string& value) {
		if (!value.empty()) os << csvEscape(value);
	}
}

uint8_t Wonder::wondersBuilt = 0;

uint8_t Wonder::getWondersBuilt() { return wondersBuilt; }
uint8_t Wonder::incrementWondersBuilt() { return ++wondersBuilt; }
void Wonder::resetWondersBuilt() { wondersBuilt = 0; }

// Core Wonder-specific getters
const uint8_t& Wonder::getShieldPoints() const { return m_shieldPoints; }
const ResourceType& Wonder::getResourceProduction() const { return m_resourceProduction; }
bool Wonder::IsConstructed() const { return isConstructed; }

// Non-applicable getters return neutral defaults
const std::unordered_map<ResourceType, uint8_t>& Wonder::getResourcesProduction() const {
	static const std::unordered_map<ResourceType, uint8_t> empty{};
	return empty;
}
const std::optional<ScientificSymbolType>& Wonder::getScientificSymbols() const {
	static const std::optional<ScientificSymbolType> none{}; return none;
}
const std::optional<LinkingSymbolType>& Wonder::getHasLinkingSymbol() const {
	static const std::optional<LinkingSymbolType> none{}; return none;
}
const std::optional<LinkingSymbolType>& Wonder::getRequiresLinkingSymbol() const {
	static const std::optional<LinkingSymbolType> none{}; return none;
}
const std::unordered_map<TradeRuleType,bool>& Wonder::getTradeRules() const {
	static const std::unordered_map<TradeRuleType,bool> empty{}; return empty;
}
const Age& Wonder::getAge() const {
	// Wonders do not have an Age in this model; return a neutral default
	static const Age kNeutral = Age::NO_AGE;
	return kNeutral;
}

void Wonder::setShieldPoints(uint8_t pts) { m_shieldPoints = pts; }
void Wonder::setResourceProduction(ResourceType r) { m_resourceProduction = r; }
void Wonder::setConstructed(bool constructed) { isConstructed = constructed; }

 

WonderBuilder& WonderBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
WonderBuilder& WonderBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
WonderBuilder& WonderBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
WonderBuilder& WonderBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
WonderBuilder& WonderBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
WonderBuilder& WonderBuilder::setShieldPoints(const uint8_t& pts) { m_card.setShieldPoints(pts); return *this; }
WonderBuilder& WonderBuilder::setResourceProduction(const ResourceType& r) { m_card.setResourceProduction(r); return *this; }
WonderBuilder& WonderBuilder::setConstructed(bool constructed) { m_card.setConstructed(constructed); return *this; }
WonderBuilder& WonderBuilder::addOnPlayAction(const std::function<void()>& action, std::string actionString) { m_card.addOnPlayAction(action, std::move(actionString)); return *this; }
WonderBuilder& WonderBuilder::addOnDiscardAction(const std::function<void()>& action, std::string actionString) { m_card.addOnDiscardAction(action, std::move(actionString)); return *this; }
Wonder WonderBuilder::build() { return std::move(m_card); }

void Wonder::onDiscard()
{
	Card::onDiscard();
}

void Models::Wonder::attachUnderCard(std::unique_ptr<Models::Card> c)
{
	m_underCard = std::move(c);
}

const Models::Card* Models::Wonder::getAttachedCard() const
{
	return m_underCard.get();
}

__declspec(dllexport) std::ostream& Models::operator<<(std::ostream& os, const Wonder& card)
{
	writeEscapedField(os, card.getName());
	writeEscapedField(os, resourceMapToString(card.getResourceCost()));
	writeNumericField(os, card.getVictoryPoints());
	writeEscapedField(os, card.getCaption());
	writeEscapedField(os, ColorTypeToString(card.getColor()));

	std::string produced = card.getResourceProduction() == ResourceType::NO_RESOURCE
		? std::string{}
		: ResourceTypeToString(card.getResourceProduction());
	writeEscapedField(os, produced);

	writeNumericField(os, card.getShieldPoints());
	writeFinalEscapedField(os, actionPairVectorToString(card.getOnPlayActions()));

	return os;
}
