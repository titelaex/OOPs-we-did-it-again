export module Card; 

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

import <array>; 
import <cstdint>;
import <unordered_map>;
import <string>;
import ResourceType;
import LinkingSymbolType;
import TradeRuleType;
import CoinWorthType;
import ColorType;
import ICard;
import <iostream>;

export namespace Models
{
	export class DLL_API Card : ICard
	{
	private:
		std::string m_name;
		std::unordered_map<ResourceType, uint8_t> m_resourceCost;
		std::unordered_map<ResourceType, uint8_t> m_resourceProduction;
		uint8_t m_victoryPoints{};
		uint8_t m_shieldPoints{};
		std::array<uint8_t, 7> m_scientificSymbols = { 0, 0, 0, 0, 0, 0, 0 };
		LinkingSymbolType m_hasLinkingSymbol = LinkingSymbolType::NO_SYMBOL;
		LinkingSymbolType m_requiresLinkingSymbol = LinkingSymbolType::NO_SYMBOL;
		CoinWorthType m_coinWorth = CoinWorthType::VALUE;
		uint8_t m_coinValue = 0;
		std::unordered_map<TradeRuleType, bool> m_tradeRules;
		std::string m_caption;
		ColorType m_color{};
		bool m_isVisibile = false;
		std::string m_modelPath = "UI path placeholder";

	public:
		const std::string& GetName() const { return m_name; }
		const std::unordered_map<ResourceType, uint8_t>& GetResourceCost() const { return m_resourceCost; }
		const std::unordered_map<ResourceType, uint8_t>& GetResourceProduction() const { return m_resourceProduction; }
		uint8_t GetVictoryPoints() const { return m_victoryPoints; }
		uint8_t GetShieldPoints() const { return m_shieldPoints; }
		const std::array<uint8_t, 7>& GetScientificSymbols() const { return m_scientificSymbols; }
		LinkingSymbolType GetHasLinkingSymbol() const { return m_hasLinkingSymbol; }
		LinkingSymbolType GetRequiresLinkingSymbol() const { return m_requiresLinkingSymbol; }
		CoinWorthType GetCoinWorth() const { return m_coinWorth; }
		uint8_t GetCoinValue() const { return m_coinValue; }
		const std::unordered_map<TradeRuleType, bool>& GetTradeRules() const { return m_tradeRules; }
		const std::string& GetCaption() const { return m_caption; }
		ColorType GetColor() const { return m_color; }
		bool GetIsVisible() const { return m_isVisibile; }
		const std::string& GetModelPath() const { return m_modelPath; }

		void SetName(const std::string& name) { m_name = name; }
		void SetResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
		void SetResourceProduction(const std::unordered_map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
		void SetVictoryPoints(uint8_t victoryPoints) { m_victoryPoints = victoryPoints; }
		void SetShieldPoints(uint8_t shieldPoints) { m_shieldPoints = shieldPoints; }
		void SetScientificSymbols(const std::array<uint8_t, 7>& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
		void SetHasLinkingSymbol(LinkingSymbolType hasLinkingSymbol) { m_hasLinkingSymbol = hasLinkingSymbol; }
		void SetRequiresLinkingSymbol(LinkingSymbolType requiresLinkingSymbol) { m_requiresLinkingSymbol = requiresLinkingSymbol; }
		void SetCoinValue(uint8_t coinValue) { m_coinValue = coinValue; }
		void SetTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules) { m_tradeRules = tradeRules; }
		void SetCoinWorth(CoinWorthType coinWorth)
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
		void SetCaption(const std::string& caption) { m_caption = caption; }
		void SetColor(ColorType color) { m_color = color; }
		void SetIsVisible(bool isVisible) { m_isVisibile = isVisible; }
		void SetModelPath(const std::string& modelPath) { m_modelPath = modelPath; }

		virtual void toggleVisibility() override
		{
			m_isVisibile = !m_isVisibile;
		}
		virtual void displayCardInfo() override
		{
			std::cout << "Card Name: " << m_name << "\n";
			std::cout << "Color: " << static_cast<int>(m_color) << "\n";
			std::cout << "Caption: " << m_caption << "\n";
			std::cout << "Model Path: " << m_modelPath << "\n";
			std::cout << "Visible: " << (m_isVisibile ? "Yes" : "No") << "\n";

			std::cout << "Resource Cost:\n";
			for (const auto& [resource, amount] : m_resourceCost)
			{
				std::cout << "  - " << static_cast<int>(resource) << ": " << static_cast<int>(amount) << "\n";
			}

			std::cout << "Resource Production:\n";
			for (const auto& [resource, amount] : m_resourceProduction)
			{
				std::cout << "  - " << static_cast<int>(resource) << ": " << static_cast<int>(amount) << "\n";
			}

			std::cout << "Victory Points: " << static_cast<int>(m_victoryPoints) << "\n";
			std::cout << "Shield Points: " << static_cast<int>(m_shieldPoints) << "\n";

			std::cout << "Scientific Symbols:\n";
			for (size_t i = 0; i < m_scientificSymbols.size(); ++i)
			{
				if (m_scientificSymbols[i] > 0)
					std::cout << "  - Symbol " << i << ": " << static_cast<int>(m_scientificSymbols[i]) << "\n";
			}

			std::cout << "Linking Symbol Provided: " << static_cast<int>(m_hasLinkingSymbol) << "\n";
			std::cout << "Linking Symbol Required: " << static_cast<int>(m_requiresLinkingSymbol) << "\n";

			std::cout << "Coin Worth Type: " << static_cast<int>(m_coinWorth) << "\n";
			std::cout << "Coin Value: " << static_cast<int>(m_coinValue) << "\n";

			std::cout << "Trade Rules:\n";
			for (const auto& [rule, enabled] : m_tradeRules)
			{
				std::cout << "  - " << static_cast<int>(rule) << ": " << (enabled ? "Enabled" : "Disabled") << "\n";
			}
		}
	};
}