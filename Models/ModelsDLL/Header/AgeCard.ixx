export module Models.AgeCard;

import <unordered_map>;
import <ostream>;
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
		AgeCard() = delete;
		AgeCard(const AgeCard& other) = delete;
		AgeCard& operator=(const AgeCard& other) = delete;
		AgeCard(AgeCard&& other) = default;
		AgeCard& operator=(AgeCard&& other) = default;
		~AgeCard() = default;

		const std::unordered_map<ResourceType, uint8_t>& getResourceProduction() const;
		const uint8_t& getShieldPoints() const;
		const ScientificSymbolType& getScientificSymbols() const;
		const LinkingSymbolType& getHasLinkingSymbol() const;
		const LinkingSymbolType& getRequiresLinkingSymbol() const;
		const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		const Age& getAge() const;

		void setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction);
		void setShieldPoints(const uint8_t& shieldPoints);
		void setScientificSymbols(const ScientificSymbolType& scientificSymbols);
		void setHasLinkingSymbol(const LinkingSymbolType& hasLinkingSymbol);
		void setRequiresLinkingSymbol(const LinkingSymbolType& requiresLinkingSymbol);
		void setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		void setAge(const Age& age);
		void onDiscard() override;
		void displayCardInfo() override;
	};
	export std::ostream& operator<<(std::ostream& os, const AgeCard& card);

	export class __declspec(dllexport) AgeCardBuilder : public CardBuilder
	{
		AgeCard m_card;
	public:
		AgeCardBuilder& setName(const std::string& name);
		AgeCardBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		AgeCardBuilder& setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction);
		AgeCardBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		AgeCardBuilder& setShieldPoints(const uint8_t& shieldPoints);
		AgeCardBuilder& setCoinWorth(const CoinWorthType& coinWorth);
		AgeCardBuilder& setCoinReward (const uint8_t& coinReward);
		AgeCardBuilder& setScientificSymbols(const ScientificSymbolType& scientificSymbols);
		AgeCardBuilder& setHasLinkingSymbol(const LinkingSymbolType& hasLinkingSymbol);
		AgeCardBuilder& setRequiresLinkingSymbol(const LinkingSymbolType& requiresLinkingSymbol);
		AgeCardBuilder& setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		AgeCardBuilder& setCaption(const std::string& caption);
		AgeCardBuilder& setColor(const ColorType& color);
		AgeCardBuilder& setAge(const Age& age);
		AgeCardBuilder& addOnPlayAction(const std::function<void()>& action);
		AgeCard build();
	};
}