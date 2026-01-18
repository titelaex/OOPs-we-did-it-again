export module Models.AgeCard;

import <unordered_map>;
import <ostream>;
import <optional>;
import <functional>;
import <string>;
import <vector>;
import <utility>;
import <memory>;
import Models.Card;
import Models.ScientificSymbolType;
import Models.ResourceType;
import Models.LinkingSymbolType;
import Models.TradeRuleType;
import Models.Age;
import Models.ColorType;

namespace Models
{
	export class __declspec(dllexport) AgeCard : public Card
	{
	private: 
		std::unordered_map<ResourceType, uint8_t> m_resourceProduction{};
		uint8_t m_shieldPoints{};
		uint8_t m_coinCost{0};
		std::optional<ScientificSymbolType> m_scientificSymbols{};
		std::optional<LinkingSymbolType> m_hasLinkingSymbol{};
		std::optional<LinkingSymbolType> m_requiresLinkingSymbol{};
		std::unordered_map<TradeRuleType, bool> m_tradeRules{};
		Age m_age{Age::AGE_I};

	public:
		AgeCard() = default;
		AgeCard(const AgeCard& other) = delete;
		AgeCard& operator=(const AgeCard& other) = delete;
		AgeCard(AgeCard&& other) = default;
		AgeCard& operator=(AgeCard&& other) = default;
		~AgeCard() = default;

		virtual const std::unordered_map<ResourceType, uint8_t>& getResourcesProduction() const;
		virtual const uint8_t& getShieldPoints() const;
		virtual uint8_t getCoinCost() const;
		virtual const std::optional<ScientificSymbolType>& getScientificSymbols() const;
		virtual const std::optional<LinkingSymbolType>& getHasLinkingSymbol() const;
		virtual const std::optional<LinkingSymbolType>& getRequiresLinkingSymbol() const;
		virtual const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		virtual const Age& getAge() const;

		virtual const ResourceType& getResourceProduction() const;
		virtual bool IsConstructed() const;

		void setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction);
		void setCoinCost(uint8_t coinCost);
		void setShieldPoints(const uint8_t& shieldPoints);
		void setScientificSymbols(const std::optional<ScientificSymbolType>& scientificSymbols);
		void setHasLinkingSymbol(const std::optional<LinkingSymbolType>& hasLinkingSymbol);
		void setRequiresLinkingSymbol(const std::optional<LinkingSymbolType>& requiresLinkingSymbol);
		void setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		void setAge(const Age& age);
		void onDiscard();
		void displayCardInfo();
	};

	export __declspec(dllexport) std::ostream& operator<<(std::ostream& os, const AgeCard& card);

	export class __declspec(dllexport) AgeCardBuilder : public CardBuilder
	{
		std::unique_ptr<AgeCard> m_card{std::make_unique<AgeCard>()};
	public:
		AgeCardBuilder& setName(const std::string& name);
		AgeCardBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		AgeCardBuilder& setResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction);
		AgeCardBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		AgeCardBuilder& setShieldPoints(const uint8_t& shieldPoints);
		AgeCardBuilder& setCoinCost(uint8_t coinCost);
		AgeCardBuilder& setScientificSymbols(const std::optional<ScientificSymbolType>& scientificSymbols);
		AgeCardBuilder& setHasLinkingSymbol(const std::optional<LinkingSymbolType>& hasLinkingSymbol);
		AgeCardBuilder& setRequiresLinkingSymbol(const std::optional<LinkingSymbolType>& requiresLinkingSymbol);
		AgeCardBuilder& setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		AgeCardBuilder& setCaption(const std::string& caption);
		AgeCardBuilder& setColor(const ColorType& color);
		AgeCardBuilder& setAge(const Age& age);
		AgeCardBuilder& addOnPlayAction(const std::function<void()>& action, std::string actionString);
		AgeCardBuilder& addOnDiscardAction(const std::function<void()>& action, std::string actionString);
		AgeCard build();
	};
}