export module Models.GuildCard;

import Models.Card; // imports the Card module (provides Models::Card)
import Models.Age; // used to set card age to AGE_III
import Models.ResourceType;
import Models.CoinWorthType;
import Models.ColorType;

import <string>;
import <vector>;
import <utility>;
import <iostream>; // only for displayCardInfo override
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

		// Copy constructor (deleted because base Card is non-copyable)
		GuildCard(const GuildCard& other) = delete;

		// Move constructor (noexcept to allow container optimizations)
		GuildCard(GuildCard&& other) noexcept;

		// Copy assignment (deleted)
		GuildCard& operator=(const GuildCard& other) = delete;

		// Move assignment
		GuildCard& operator=(GuildCard&& other) noexcept;


		~GuildCard() override = default;

		 void displayCardInfo() override;
		 void onDiscard() override;
	};

	// Builder for GuildCard
	export class __declspec(dllexport) GuildCardBuilder : public CardBuilder
	{
		GuildCard m_card;
	public:
		GuildCardBuilder& setName(const std::string& name);
		GuildCardBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		GuildCardBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		GuildCardBuilder& setCoinWorth(const CoinWorthType& coinWorth);
		GuildCardBuilder& setCoinReward(const uint8_t& coinReward);
		GuildCardBuilder& setCaption(const std::string& caption);
		GuildCardBuilder& setColor(const ColorType& color);
		GuildCardBuilder& addOnPlayAction(const std::function<void()>& action);
		GuildCardBuilder& addOnDiscardAction(const std::function<void()>& action);
		GuildCard build();
	};
}
