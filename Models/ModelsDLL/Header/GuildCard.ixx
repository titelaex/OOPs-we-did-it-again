export module Models.GuildCard;

import Models.Card;
import Models.Age; 
import Models.ResourceType;
import Models.CoinWorthType;
import Models.ColorType;

import <string>;
import <vector>;
import <utility>;
import <iostream>;
import <fstream>;
import <sstream>;
import <stdexcept>;
import <unordered_map>;

export namespace Models
{
	export class __declspec(dllexport) GuildCard : public Card
	{
	private:
		static const Age m_age = Age::AGE_III;
	public:
		GuildCard() = default;

		GuildCard(const GuildCard& other) = delete;

		GuildCard(GuildCard&& other) noexcept;

		GuildCard& operator=(const GuildCard& other) = delete;

		GuildCard& operator=(GuildCard&& other) noexcept;


		~GuildCard() override = default;

		virtual const std::unordered_map<ResourceType, uint8_t>& getResourcesProduction() const;
		virtual const uint8_t& getShieldPoints() const;
		virtual const std::optional<ScientificSymbolType>& getScientificSymbols() const;
		virtual const std::optional<LinkingSymbolType>& getHasLinkingSymbol() const;
		virtual const std::optional<LinkingSymbolType>& getRequiresLinkingSymbol() const;
		virtual const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		virtual const Age& getAge() const;

		virtual const ResourceType& getResourceProduction() const;
		virtual bool IsConstructed() const;

		 void displayCardInfo() override;
		 void onDiscard() override;
	};

	export class __declspec(dllexport) GuildCardBuilder : public CardBuilder
	{
		GuildCard m_card;
	public:
		GuildCardBuilder& setName(const std::string& name);
		GuildCardBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		GuildCardBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		GuildCardBuilder& setCaption(const std::string& caption);
		GuildCardBuilder& setColor(const ColorType& color);
		GuildCardBuilder& addOnPlayAction(const std::function<void()>& action, std::string actionString);
		GuildCardBuilder& addOnDiscardAction(const std::function<void()>& action, std::string actionString);
		GuildCard build();
	};
}
