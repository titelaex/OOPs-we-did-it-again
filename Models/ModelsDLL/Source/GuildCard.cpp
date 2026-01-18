module Models.GuildCard;

import <iostream>;
import Models.GuildCard;
import Models.ResourceType;
import Models.Age;
import Models.TradeRuleType;
import <optional>;
import <unordered_map>;
import Models.Card;
import <ostream>;
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
		out.reserve(s.size() + 2);
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

GuildCard::GuildCard(GuildCard&& other) noexcept
	: Card(std::move(other)) {}
GuildCard& GuildCard::operator=(GuildCard&& other) noexcept { if (this != &other) { Card::operator=(std::move(other));  } return *this; }

void GuildCard::onDiscard()
{
    Card::onDiscard();
}

GuildCardBuilder& GuildCardBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
GuildCardBuilder& GuildCardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
GuildCardBuilder& GuildCardBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
GuildCardBuilder& GuildCardBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
GuildCardBuilder& GuildCardBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
GuildCardBuilder& GuildCardBuilder::addOnPlayAction(const std::function<void()>& action, std::string actionString) { m_card.addOnPlayAction(action, std::move(actionString)); return *this; }
GuildCardBuilder& GuildCardBuilder::addOnDiscardAction(const std::function<void()>& action, std::string actionString) { m_card.addOnDiscardAction(action, std::move(actionString)); return *this; }
GuildCard GuildCardBuilder::build() { return std::move(m_card); }

const std::unordered_map<ResourceType,uint8_t>& GuildCard::getResourcesProduction() const {
 static const std::unordered_map<ResourceType,uint8_t> empty{}; return empty;
}
const uint8_t& GuildCard::getShieldPoints() const {
 static const uint8_t zero =0; return zero;
}
const std::optional<ScientificSymbolType>& GuildCard::getScientificSymbols() const {
 static const std::optional<ScientificSymbolType> none{}; return none;
}
const std::optional<LinkingSymbolType>& GuildCard::getHasLinkingSymbol() const {
 static const std::optional<LinkingSymbolType> none{}; return none;
}
const std::optional<LinkingSymbolType>& GuildCard::getRequiresLinkingSymbol() const {
 static const std::optional<LinkingSymbolType> none{}; return none;
}
const std::unordered_map<TradeRuleType,bool>& GuildCard::getTradeRules() const {
 static const std::unordered_map<TradeRuleType,bool> empty{}; return empty;
}
const Age& GuildCard::getAge() const {
	static const Age kNeutral = Age::NO_AGE;
	return kNeutral;
}
const ResourceType& GuildCard::getResourceProduction() const {
 static const ResourceType kNone = ResourceType::NO_RESOURCE; return kNone;
}
bool GuildCard::IsConstructed() const { static const bool kFalse=false; return kFalse; }

__declspec(dllexport) std::ostream& operator<<(std::ostream& os, const GuildCard& card)
{
	writeEscapedField(os, card.getName());
	writeEscapedField(os, resourceMapToString(card.getResourceCost()));
	writeNumericField(os, card.getVictoryPoints());
	writeEscapedField(os, card.getCaption());
	writeEscapedField(os, ColorTypeToString(card.getColor()));
	writeFinalEscapedField(os, actionPairVectorToString(card.getOnPlayActions()));

	return os;
}