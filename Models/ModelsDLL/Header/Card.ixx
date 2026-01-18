export module Models.Card; 

import <ostream>;
import <functional>;
import <array>; 
import <cstdint>;
import <unordered_map>;
import <string>;
import <optional>;
import <vector>;
import <utility>;
import Models.ResourceType;
import Models.CoinWorthType;
import Models.ColorType;
import Models.ICard;
import <iostream>;
import Models.ScientificSymbolType;
import Models.LinkingSymbolType;
import Models.TradeRuleType;
import Models.Age;

export namespace Models
{
	export class __declspec(dllexport) Card : ICard
	{
	private:
		std::string m_name;
		std::unordered_map<ResourceType, uint8_t> m_resourceCost{};
		uint8_t m_victoryPoints{};
		std::string m_caption{};
		ColorType m_color{};
		std::vector<std::pair<std::function<void()>, std::string>> m_onPlayActions{};
		std::vector<std::pair<std::function<void()>, std::string>> m_onDiscardActions{};
		bool m_isVisible{ false };
		bool m_isAvailable{ false };

	public:
		Card() = default;
		Card(const Card& other) = delete;
		virtual Card& operator=(const Card& other) = delete;
		Card(Card&& other);
		virtual Card& operator=(Card&& other);
		virtual ~Card() = default;

		virtual void onPlay();
		virtual void onDiscard();

		const std::string& getName() const;
		const std::unordered_map<ResourceType, uint8_t>& getResourceCost() const;
		const uint8_t& getVictoryPoints() const;
		const std::string& getCaption() const;
		const ColorType& getColor() const;
		const std::vector<std::pair<std::function<void()>, std::string>>& getOnPlayActions() const;
		const std::vector<std::pair<std::function<void()>, std::string>>& getOnDiscardActions() const;
		const bool& isVisible() const;
		const bool& isAvailable() const;

		virtual const std::unordered_map<ResourceType, uint8_t>& getResourcesProduction() const;
		virtual const uint8_t& getShieldPoints() const;
		virtual const std::optional<ScientificSymbolType>& getScientificSymbols() const;
		virtual const std::optional<LinkingSymbolType>& getHasLinkingSymbol() const;
		virtual const std::optional<LinkingSymbolType>& getRequiresLinkingSymbol() const;
		virtual const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		virtual const Age& getAge() const;

		virtual const ResourceType& getResourceProduction() const;
		virtual bool IsConstructed() const;

		void setName(const std::string& name);
		void setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		void setVictoryPoints(const uint8_t& victoryPoints);
		void setCaption(const std::string& caption);
		void setColor(const ColorType& color);
		void addOnPlayAction(const std::function<void()>& action, std::string actionString);
		void addOnDiscardAction(const std::function<void()>& action, std::string actionString);
		void setIsVisible(const bool& isVisible);
		void setIsAvailable(const bool& isAvailable);

	};

	export class __declspec(dllexport) CardBuilder
	{
		Card m_card;
	public:
		CardBuilder& setName(const std::string& name);
		CardBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		CardBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		CardBuilder& setCaption(const std::string& caption);
		CardBuilder& setColor(const ColorType& color);
		CardBuilder& addOnPlayAction(const std::function<void()>& action, std::string actionString);
		CardBuilder& addOnDiscardAction(const std::function<void()>& action, std::string actionString);
		Card build();

	};

	export __declspec(dllexport) std::ostream& operator<<(std::ostream& out, const Card& card);
}