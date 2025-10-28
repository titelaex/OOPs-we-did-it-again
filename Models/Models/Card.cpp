#include "Card.h"
#include <algorithm>

using namespace Models;

const std::map<ResourceType, uint8_t>& Card::GetResourceCost() const { return m_resourceCost; }
const std::map<ResourceType, uint8_t>& Card::GetResourceProduction() const { return m_resourceProduction; }
uint8_t Card::GetVictoryPoints() const { return m_victoryPoints; }
uint8_t Card::GetShieldPoints() const { return m_shieldPoints; }
const std::array<uint8_t,7>& Card::GetScientificSymbols() const { return m_scientificSymbols; }
LinkingSymbolType Card::GetHasLinkingSymbol() const { return m_hasLinkingSymbol; }
LinkingSymbolType Card::GetRequiresLinkingSymbol() const { return m_requiresLinkingSymbol; }
CoinWorthType Card::GetCoinWorth() const { return m_coinWorth; }
uint8_t Card::GetCoinValue() const { return m_coinValue; }
const std::map<TradeRuleType, bool>& Card::GetTradeRules() const { return m_tradeRules; }
const std::string& Card::GetCaption() const { return m_caption; }
ColorType Card::GetColor() const { return m_color; }
bool Card::GetIsVisible() const { return m_isVisibile; }
const std::string& Card::GetModelPath() const { return m_modelPath; }

void Card::SetResourceCost(const std::map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
void Card::SetResourceProduction(const std::map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
void Card::SetVictoryPoints(uint8_t victoryPoints) { m_victoryPoints = victoryPoints; }
void Card::SetShieldPoints(uint8_t shieldPoints) { m_shieldPoints = shieldPoints; }
void Card::SetScientificSymbols(const std::array<uint8_t,7>& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
void Card::SetHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
void Card::SetRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
void Card::SetCoinValue(uint8_t coinValue) { m_coinValue = coinValue; }
void Card::SetTradeRules(const std::map<TradeRuleType, bool>& tradeRules) { m_tradeRules = tradeRules; }
void Card::SetCoinWorth(CoinWorthType coinWorth)
{
	m_coinWorth = coinWorth;
	switch (coinWorth)
	{
	case CoinWorthType::VALUE:
		break;
	case CoinWorthType::WONDER:
		SetCoinValue(2); // wonders worth2 coins
		break;
	case CoinWorthType::GREY:
		SetCoinValue(3);
		break;
	case CoinWorthType::BROWN:
		SetCoinValue(2);
		break;
	case CoinWorthType::YELLOW:
		SetCoinValue(1);
		break;
	case CoinWorthType::RED:
		SetCoinValue(1);
		break;
	default:
		break;
	}
}
void Card::SetCaption(const std::string& caption) { m_caption = caption; }
void Card::SetColor(ColorType color) { m_color = color; }
void Card::SetIsVisible(bool isVisible) { m_isVisibile = isVisible; }
void Card::SetModelPath(const std::string& modelPath) { m_modelPath = modelPath; }
