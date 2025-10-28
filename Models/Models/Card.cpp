#include "Card.h"
#include "coinWorth.h"
using namespace Models;
class Card
{
private:
	std::map<ResourceType, uint8_t> m_resourceCost;
	std::map<ResourceType, uint8_t> m_resourceProduction;
	uint8_t m_victoryPoints;
	uint8_t m_shieldPoints;
	std::array<uint8_t, 7> m_scientificSymbols = { 0, 0, 0, 0, 0, 0, 0 };
	linkingSymbolType m_hasLinkingSymbol = linkingSymbolType::NO_SYMBOL;
	linkingSymbolType m_requiresLinkingSymbol = linkingSymbolType::NO_SYMBOL;
	coinWorthType m_coinWorth = coinWorthType::VALUE;
	uint8_t m_coinValue = 0;

public:
	const std::map<ResourceType, uint8_t>& GetResourceCost() const { return m_resourceCost; }
	const std::map<ResourceType, uint8_t>& GetResourceProduction() const { return m_resourceProduction; }
	uint8_t GetVictoryPoints() const { return m_victoryPoints; }
	uint8_t GetShieldPoints() const { return m_shieldPoints; }
	const std::array<uint8_t, 7>& GetScientificSymbols() const { return m_scientificSymbols; }
	linkingSymbolType GetHasLinkingSymbol() const { return m_hasLinkingSymbol; }
	linkingSymbolType GetRequiresLinkingSymbol() const { return m_requiresLinkingSymbol; }
	coinWorthType GetCoinWorth() const { return m_coinWorth; }
	uint8_t GetCoinValue() const { return m_coinValue; }

	void SetResourceCost(const std::map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
	void SetResourceProduction(const std::map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
	void SetVictoryPoints(uint8_t victoryPoints) { m_victoryPoints = victoryPoints; }
	void SetShieldPoints(uint8_t shieldPoints) { m_shieldPoints = shieldPoints; }
	void SetScientificSymbols(const std::array<uint8_t, 7>& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
	void SetHasLinkingSymbol(linkingSymbolType hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
	void SetRequiresLinkingSymbol(linkingSymbolType requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
	void SetCoinValue(uint8_t coinValue) { m_coinValue = coinValue; }
	void SetCoinWorth(coinWorthType coinWorth) 
	{ 
		m_coinWorth = coinWorth;
		switch (coinWorth)
		{
		case Models::coinWorthType::VALUE:
			break;
		case Models::coinWorthType::WONDER:
			SetCoinValue(2); // minunile au valoare de 2 monede
			break;
		case Models::coinWorthType::GREY:
			SetCoinValue(3);
			break;
		case Models::coinWorthType::BROWN:
			SetCoinValue(2);
			break;
		case Models::coinWorthType::YELLOW:
			SetCoinValue(1);
			break;
		case Models::coinWorthType::RED:
			SetCoinValue(1);
			break;
		default:
			break;
		}
	}
	
};
