export module Models.GuildCard;

import Models.Card; // imports the Card module (provides Models::Card)
import Models.Age; // used to set card age to AGE_III

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
		 std::vector<std::string> m_scoringRules;
		 std::string m_guildName;
		static const Age m_age = Age::AGE_III;
	public:
		 // Default ctor: ensure the card age is Age III (guilds belong to Age III)
		 //GuildCard();

		 // Construct from guild metadata (full ctor)
		 GuildCard(const std::string& name,
			 const std::unordered_map<ResourceType, uint8_t>& resourceCost,
			 uint8_t victoryPoints,
			 CoinWorthType coinWorth,
			 uint8_t coinReward,
			 const std::string& caption,
			 ColorType color,
			 bool isVisible,
			 const std::string& modelPath,
			 const std::vector<std::string>& scoringRules);

		 // Construct from minimal CSV fields (name + scoring rules)
		 GuildCard(const std::string& name, const std::vector<std::string>& scoringRules);

		 // Copy constructor (deleted because base Card is non-copyable)
		 GuildCard(const GuildCard& other) = default;

		 // Move constructor (noexcept to allow container optimizations)
		 GuildCard(GuildCard&& other) noexcept;

		 // Copy assignment (deleted)
		 GuildCard& operator=(const GuildCard& other) = default;

		 // Move assignment
		 GuildCard& operator=(GuildCard&& other) noexcept;

		
		 ~GuildCard() override = default;

		 const std::string& GetGuildName() const noexcept;
		 const std::vector<std::string>& GetScoringRules() const noexcept;
		 void SetGuildName(const std::string& name);
		 void SetScoringRules(const std::vector<std::string>& rules);

		 void toggleVisibility() override;
		 void toggleAccessibility() override;
		 void displayCardInfo() override;

	};

	export std::vector<GuildCard> LoadGuildCardsFromCSV(const std::string& path);
}
