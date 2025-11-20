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
		 GuildCard(const std::vector<std::string>& scoringRules);
		 // + parametrii card

		
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

		 const std::vector<std::string>& GetScoringRules() const noexcept;
		 void SetScoringRules(const std::vector<std::string>& rules);

		 void displayCardInfo() override;
	private:
		 std::vector<std::string> m_scoringRules;
	};

	export std::vector<GuildCard> LoadGuildCardsFromCSV(const std::string& path);
}
