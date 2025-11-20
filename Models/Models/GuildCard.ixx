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

export namespace Models
{
	export class __declspec(dllexport) GuildCard : public Card
	{
	public:
		 // Default ctor: ensure the card age is Age III (guilds belong to Age III)
		 GuildCard();

		 // Construct from guild metadata
		 GuildCard(const std::string& guildName, const std::vector<std::string>& scoringRules);

		 // Rule of Five: explicitly define special members so ownership and polymorphic
		 // behaviour are clear and maintainable. Members are standard types so default
		 // implementations would work, but providing explicit definitions documents intent
		 // and forwards calls to the base class where appropriate.

		 // Copy constructor
		 GuildCard(const GuildCard& other);

		 // Move constructor (noexcept to allow container optimizations)
		 GuildCard(GuildCard&& other) noexcept;

		 // Copy assignment
		 GuildCard& operator=(const GuildCard& other);

		 // Move assignment
		 GuildCard& operator=(GuildCard&& other) noexcept;

		 // Virtual destructor: default is fine since members are RAII types
		 virtual ~GuildCard() override = default;

		 // Accessors
		 const std::string& GetGuildName() const noexcept;
		 const std::vector<std::string>& GetScoringRules() const noexcept;
		 void SetGuildName(const std::string& name);
		 void SetScoringRules(const std::vector<std::string>& rules);

		 // Override display to include guild-specific info; still calls Card::displayCardInfo()
		 virtual void displayCardInfo() override;
	private:
		 // Guild-specific data: human-readable name and a set of scoring rule descriptions.
		 // You may later replace scoring rules with a structured rule object or callback
		 // that applies points to a Player; strings are a simple first step.
		 std::string m_guildName;
		 std::vector<std::string> m_scoringRules;
	};

	// Load guild cards from a CSV file. Expected header: guild_name,scoring_rules,description
	// scoring_rules are semicolon-separated; description is optional and ignored by loader currently.
	export std::vector<GuildCard> LoadGuildCardsFromCSV(const std::string& path);
}
