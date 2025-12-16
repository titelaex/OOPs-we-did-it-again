module Models.GuildCard;

import <iostream>;
import Models.GuildCard;
import Models.ResourceType;
import Models.Age;
import Models.TradeRuleType;
import <optional>;
import <unordered_map>;


using namespace Models;

GuildCard::GuildCard(GuildCard&& other) noexcept
	: Card(std::move(other)) {}
GuildCard& GuildCard::operator=(GuildCard&& other) noexcept { if (this != &other) { Card::operator=(std::move(other));  } return *this; }

void GuildCard::displayCardInfo() {
	std::cout << "\n";
	Card::displayCardInfo();
}

void GuildCard::onDiscard()
{
    Card::onDiscard();
}

GuildCardBuilder& GuildCardBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
GuildCardBuilder& GuildCardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
GuildCardBuilder& GuildCardBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
GuildCardBuilder& GuildCardBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
GuildCardBuilder& GuildCardBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
GuildCardBuilder& GuildCardBuilder::addOnPlayAction(const std::function<void()>& action) { m_card.addOnPlayAction(action); return *this; }
GuildCardBuilder& GuildCardBuilder::addOnDiscardAction(const std::function<void()>& action) { m_card.addOnDiscardAction(action); return *this; }
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
const Age& GuildCard::getAge() const { return GuildCard::m_age; }
const ResourceType& GuildCard::getResourceProduction() const {
 static const ResourceType kNone = ResourceType::NO_RESOURCE; return kNone;
}
bool GuildCard::IsConstructed() const { static const bool kFalse=false; return kFalse; }