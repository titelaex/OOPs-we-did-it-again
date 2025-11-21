export module Models.Card; 

import <array>; 
import <cstdint>;
import <unordered_map>;
import <string>;
import Models.ResourceType;
import Models.CoinWorthType;
import Models.ColorType;
import Models.ICard;
import <iostream>;

export namespace Models
{
	export class __declspec(dllexport) Card : ICard
	{
	private:
		std::string m_name;
		std::unordered_map<ResourceType, uint8_t> m_resourceCost;
		uint8_t m_victoryPoints{};
		CoinWorthType m_coinWorth = CoinWorthType::VALUE;
		uint8_t m_coinReward = 0;
		std::string m_caption;
		ColorType m_color{};
		bool m_isVisibile = false;
		bool m_isAccessible = false;
		std::string m_modelPath = "UI path placeholder";

	public:
		Card() = default;
		Card(const Card& other) = default;
		virtual Card& operator=(const Card& other) = default;
		Card(Card&& other) noexcept;
		virtual Card& operator=(Card&& other) noexcept;
		virtual ~Card() = default;

		const std::string& GetName() const;
		const std::unordered_map<ResourceType, uint8_t>& GetResourceCost() const;
		const std::unordered_map<ResourceType, uint8_t>& GetResourceProduction() const;
		uint8_t GetVictoryPoints() const;
		uint8_t GetShieldPoints() const;
		uint8_t GetCoinCost() const;
		const std::array<uint8_t, 7>& GetScientificSymbols() const;
		LinkingSymbolType GetHasLinkingSymbol() const;
		LinkingSymbolType GetRequiresLinkingSymbol() const;
		CoinWorthType GetCoinWorth() const;
		uint8_t GetCoinValue() const;
		const std::unordered_map<TradeRuleType, bool>& GetTradeRules() const;
		const std::string& GetCaption() const;
		ColorType GetColor() const;
		bool GetIsVisible() const;
		const std::string& GetModelPath() const;

		void SetName(const std::string& name);
		void SetResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		void SetResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction);
		void SetVictoryPoints(uint8_t victoryPoints);
		void SetShieldPoints(uint8_t shieldPoints);
		void SetCoinCost(uint8_t cost);
		void SetScientificSymbols(const std::array<uint8_t, 7>& scientificSymbols);
		void SetHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol);
		void SetRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol);
		void SetCoinValue(uint8_t coinValue);
		void SetTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		void SetCoinWorth(CoinWorthType coinWorth);
		void SetCaption(const std::string& caption);
		void SetColor(ColorType color);
		void SetIsVisible(bool isVisible);
		void SetModelPath(const std::string& modelPath);
		void SetAge(const Age& age);
		
		virtual void toggleVisibility() override;
		virtual void toggleAccessibility() override;
		virtual void displayCardInfo() override;
	};
}