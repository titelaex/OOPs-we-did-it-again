export module Models.Wonder;

import Models.ResourceType;
import Models.Token;
import Models.Card;
import <string>;
import <vector>;
import <random>;
import <algorithm>;
import <iterator>;
import <bitset>;

namespace Models
{
	export class __declspec(dllexport) Wonder : public Card
	{
	private:
		uint8_t m_shieldPoints{ 0 };
		ResourceType m_resourceProduction{};
		bool isConstructed{ false };

	public:
		static uint8_t wondersBuilt;
		static constexpr uint8_t MaxWonders = 7;
		static uint8_t getWondersBuilt();
		static uint8_t incrementWondersBuilt();
		static void resetWondersBuilt();

	public:

		Wonder() = default;
		Wonder(const Wonder& other) = delete;
		Wonder& operator=(const Wonder& other) = delete;
		Wonder(Wonder&& other) = default;
		Wonder& operator=(Wonder&& other) = default;
		~Wonder() = default;

		const uint8_t& getShieldPoints() const override;
		const ResourceType& getResourceProduction() const override;
		bool IsConstructed() const override;

		virtual const std::unordered_map<ResourceType, uint8_t>& getResourcesProduction() const;
		virtual const std::optional<ScientificSymbolType>& getScientificSymbols() const;
		virtual const std::optional<LinkingSymbolType>& getHasLinkingSymbol() const;
		virtual const std::optional<LinkingSymbolType>& getRequiresLinkingSymbol() const;
		virtual const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		virtual const Age& getAge() const;

		void setShieldPoints(uint8_t pts);
		void setResourceProduction(ResourceType r);
		void setConstructed(bool constructed);

		void displayCardInfo() override;
		void onDiscard() override;

	};
	export std::ostream& operator<<(std::ostream& os, const Wonder& card);

	export class __declspec(dllexport) WonderBuilder : public CardBuilder
	{
		Wonder m_card;
	public:
		WonderBuilder& setName(const std::string& name);
		WonderBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		WonderBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		WonderBuilder& setCaption(const std::string& caption);
		WonderBuilder& setColor(const ColorType& color);
		WonderBuilder& setShieldPoints(const uint8_t& pts);
		WonderBuilder& setResourceProduction(const ResourceType& r);
		WonderBuilder& setConstructed(bool constructed);
		WonderBuilder& addOnPlayAction(const std::function<void()>& action);
		WonderBuilder& addOnDiscardAction(const std::function<void()>& action);
		Wonder build();
	};
}