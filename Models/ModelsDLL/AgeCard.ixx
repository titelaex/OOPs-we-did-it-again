export module Models.AgeCard;

import <unordered_map>;
import Models.Card;
import Models.ScientificSymbolType;
import Models.ResourceType;
import Models.LinkingSymbolType;
import Models.TradeRuleType;
import Models.Age;

namespace Models
{
	export class __declspec(dllexport) AgeCard : public Card
	{
	private: 
		std::unordered_map<ResourceType, uint8_t> m_resourceProduction;
		uint8_t m_shieldPoints{};
		ScientificSymbolType m_scientificSymbols=ScientificSymbolType::NO_SYMBOL;
		LinkingSymbolType m_hasLinkingSymbol = LinkingSymbolType::NO_SYMBOL;
		LinkingSymbolType m_requiresLinkingSymbol = LinkingSymbolType::NO_SYMBOL;
		std::unordered_map<TradeRuleType, bool> m_tradeRules;
		Age m_age{};

	public:
		AgeCard() = default;
		AgeCard(const AgeCard& other) = default;
		AgeCard& operator=(const AgeCard& other) = default;
		AgeCard(AgeCard&& other) noexcept;
		AgeCard& operator=(AgeCard&& other) noexcept;
		~AgeCard() = default;
		const std::unordered_map<ResourceType, uint8_t>& getResourceProduction() const;
		uint8_t getShieldPoints() const;
		ScientificSymbolType getScientificSymbols() const;
		LinkingSymbolType getHasLinkingSymbol() const;
		LinkingSymbolType getRequiresLinkingSymbol() const;
		const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		Age getAge() const;

		void setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction);
		void setShieldPoints(uint8_t shieldPoints);
		void setScientificSymbols(ScientificSymbolType scientificSymbols);
		void setHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol);
		void setRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol);
		void setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		void setAge(const Age& age);
	};
}