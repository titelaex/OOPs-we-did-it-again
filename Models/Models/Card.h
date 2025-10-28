#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include "ResourceType.h"
#include "LinkingSymbolType.h"
#include "TradeRuleType.h"
#include "CoinWorthType.h"
#include "ColorType.h"

namespace Models
{
	class Card
	{
	private:
		std::map<ResourceType, uint8_t> m_resourceCost;
		std::map<ResourceType, uint8_t> m_resourceProduction;
		uint8_t m_victoryPoints;
		uint8_t m_shieldPoints;
		std::array<uint8_t, 7> m_scientificSymbols = { 0, 0, 0, 0, 0, 0, 0 }; 
		LinkingSymbolType m_hasLinkingSymbol = LinkingSymbolType::NO_SYMBOL;
		LinkingSymbolType m_requiresLinkingSymbol = LinkingSymbolType::NO_SYMBOL;
		CoinWorthType m_coinWorth = CoinWorthType::VALUE;
		uint8_t m_coinValue = 0;
		std::map<TradeRuleType, bool> m_tradeRules;
		std::string m_caption;
		ColorType m_color;
		bool m_isVisibile = false;
		std::string m_modelPath = "UI path placeholder";

	public:
		const std::map<ResourceType, uint8_t>& GetResourceCost() const;
		const std::map<ResourceType, uint8_t>& GetResourceProduction() const;
		uint8_t GetVictoryPoints() const;
		uint8_t GetShieldPoints() const;
		const std::array<uint8_t, 7>& GetScientificSymbols() const;
		LinkingSymbolType GetHasLinkingSymbol() const;
		LinkingSymbolType GetRequiresLinkingSymbol() const;
		CoinWorthType GetCoinWorth() const;
		uint8_t GetCoinValue() const;
		const std::map<TradeRuleType, bool>& GetTradeRules() const;
		const std::string& GetCaption() const;
		ColorType GetColor() const;
		bool GetIsVisible() const;
		const std::string& GetModelPath() const;

		void SetResourceCost(const std::map<ResourceType, uint8_t>& resourceCost);
		void SetResourceProduction(const std::map<ResourceType, uint8_t>& resourceProduction);
		void SetVictoryPoints(uint8_t victoryPoints);
		void SetShieldPoints(uint8_t shieldPoints);
		void SetScientificSymbols(const std::array<uint8_t, 7>& scientificSymbols);
		void SetHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol);
		void SetRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol);
		void SetCoinWorth(CoinWorthType coinWorth);
		void SetCoinValue(uint8_t coinValue);
		void SetTradeRules(const std::map<TradeRuleType, bool>& tradeRules);
		void SetCaption(const std::string& caption);
		void SetColor(ColorType color);
		void SetIsVisible(bool isVisible);
		void SetModelPath(const std::string& modelPath);

	};
}

