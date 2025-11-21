module Models.AgeCard;

import Models.AgeCard;
import Models.Card;
import Models.ScientificSymbolType;
import Models.ResourceType;
import Models.LinkingSymbolType;
import Models.TradeRuleType;
import Models.Age;

using namespace Models;

AgeCard::AgeCard(AgeCard&& other) noexcept = default;
AgeCard& AgeCard::operator=(AgeCard&& other) noexcept = default;

const std::unordered_map<ResourceType, uint8_t>& AgeCard::getResourceProduction() const {
    return m_resourceProduction;
}

uint8_t AgeCard::getShieldPoints() const {
    return m_shieldPoints;
}

ScientificSymbolType AgeCard::getScientificSymbols() const {
    return m_scientificSymbols;
}

LinkingSymbolType AgeCard::getHasLinkingSymbol() const {
    return m_hasLinkingSymbol;
}

LinkingSymbolType AgeCard::getRequiresLinkingSymbol() const {
    return m_requiresLinkingSymbol;
}

const std::unordered_map<TradeRuleType, bool>& AgeCard::getTradeRules() const {
    return m_tradeRules;
}

Age AgeCard::getAge() const {
    return m_age;
}

void AgeCard::setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) {
    m_resourceProduction = resourceProduction;
}

void AgeCard::setShieldPoints(uint8_t shieldPoints) {
    m_shieldPoints = shieldPoints;
}

void AgeCard::setScientificSymbols(ScientificSymbolType scientificSymbols) {
    m_scientificSymbols = scientificSymbols;
}

void AgeCard::setHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol) {
    m_hasLinkingSymbol = hasLinkingSymbol;
}

void AgeCard::setRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol) {
    m_requiresLinkingSymbol = requiresLinkingSymbol;
}

void AgeCard::setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) {
    m_tradeRules = tradeRules;
}

void AgeCard::setAge(const Age& age) {
    m_age = age;
}

