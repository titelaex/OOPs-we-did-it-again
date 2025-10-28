#pragma once
#include <array>
#include <cstdint>
#include <map>
#include "Resources.h"
#include "linkingSymbols.h"

namespace Models
{
	class Card
	{
	private:
		std::map<ResourceType, uint8_t> m_resourceCost;
		std::map<ResourceType, uint8_t> m_resourceProduction;
		uint8_t m_victoryPoints : 1;
		uint8_t m_shieldPoints : 1;
		std::array<uint8_t, 7> m_scientificSymbols = { 0, 0, 0, 0, 0, 0, 0 }; 
		linkingSymbolType m_hasLinkingSymbol = linkingSymbolType::NO_SYMBOL;
		linkingSymbolType m_requiresLinkingSymbol = linkingSymbolType::NO_SYMBOL;

	public:
		const std::map<ResourceType, uint8_t>& GetResourceCost() const;
		const std::map<ResourceType, uint8_t>& GetResourceProduction() const;
		uint8_t GetVictoryPoints() const;
		uint8_t GetShieldPoints() const;
		const std::array<uint8_t, 7>& GetScientificSymbols() const;
		linkingSymbolType GetHasLinkingSymbol() const;
		linkingSymbolType GetRequiresLinkingSymbol() const;

		void SetResourceCost(const std::map<ResourceType, uint8_t>& resourceCost);
		void SetResourceProduction(const std::map<ResourceType, uint8_t>& resourceProduction);
		void SetVictoryPoints(uint8_t victoryPoints);
		void SetShieldPoints(uint8_t shieldPoints);
		void SetScientificSymbols(const std::array<uint8_t, 7>& scientificSymbols);
		void SetHasLinkingSymbol(linkingSymbolType hasLinkingSymbol);
		void SetRequiresLinkingSymbol(linkingSymbolType requiresLinkingSymbol);

	};
}

