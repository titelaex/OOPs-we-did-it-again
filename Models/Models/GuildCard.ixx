export module GuildCard;

// GuildCard module: a concrete Card subtype representing Guild cards (Age III)
// Design notes (why derive from Card and not ICard):
// - Guild cards are a specialized kind of Card with additional fields (guild name, scoring rules).
// - Deriving from Card preserves all existing Card behaviour (cost, production, display, etc.)
// and allows reuse of Card's members and helpers. Deriving directly from ICard would
// force GuildCard to reimplement a lot of Card behaviour or to duplicate data.

import Card; // imports the Card module (provides Models::Card)
import Age; // used to set card age to AGE_III

import <string>;
import <vector>;
import <utility>;
import <iostream>; // only for displayCardInfo override


export namespace Models
{
 export class __declspec(dllexport) GuildCard : public Card
 {
 public:
	 // Default ctor: ensure the card age is Age III (guilds belong to Age III)
	 GuildCard()
	 {
		 m_age = Age::AGE_III;
	 }

	 // Construct from guild metadata
	 GuildCard(const std::string& guildName, const std::vector<std::string>& scoringRules)
		 : m_guildName(guildName), m_scoringRules(scoringRules)
	 {
		 // Ensure Age is set to Age III for guild cards
		 m_age = Age::AGE_III;
	 }

	 // Rule of Five: explicitly define special members so ownership and polymorphic
	 // behaviour are clear and maintainable. Members are standard types so default
	 // implementations would work, but providing explicit definitions documents intent
	 // and forwards calls to the base class where appropriate.

	 // Copy constructor
	 GuildCard(const GuildCard& other)
		 : Card(other) // copy base part
		 , m_guildName(other.m_guildName)
		 , m_scoringRules(other.m_scoringRules)
	 {
		 // Age already copied by Card(other)
	 }

	 // Move constructor (noexcept to allow container optimizations)
	 GuildCard(GuildCard&& other) noexcept
		 : Card(std::move(other))
		 , m_guildName(std::move(other.m_guildName))
		 , m_scoringRules(std::move(other.m_scoringRules))
	 {
	 }

	 // Copy assignment
	 GuildCard& operator=(const GuildCard& other)
	 {
		 if (this != &other)
		 {
			 Card::operator=(other); // copy-assign base
			 m_guildName = other.m_guildName;
			 m_scoringRules = other.m_scoringRules;
		 }
		 return *this;
	 }

	 // Move assignment
	 GuildCard& operator=(GuildCard&& other) noexcept
	 {
		 if (this != &other)
		 {
			 Card::operator=(std::move(other)); // move-assign base
			 m_guildName = std::move(other.m_guildName);
			 m_scoringRules = std::move(other.m_scoringRules);
		 }
		 return *this;
	 }

	 // Virtual destructor: default is fine since members are RAII types
	 virtual ~GuildCard() override = default;

	 // Accessors
	 const std::string& GetGuildName() const noexcept { return m_guildName; }
	 const std::vector<std::string>& GetScoringRules() const noexcept { return m_scoringRules; }

	 void SetGuildName(const std::string& name) { m_guildName = name; }
	 void SetScoringRules(const std::vector<std::string>& rules) { m_scoringRules = rules; }

	 // Override display to include guild-specific info; still calls Card::displayCardInfo()
	 virtual void displayCardInfo() override
	 {
		 Card::displayCardInfo(); // print base card data
		 std::cout << "Guild Name: " << m_guildName << "\n";
		 std::cout << "Scoring Rules:\n";
		 for (const auto& r : m_scoringRules)
			 std::cout << " - " << r << "\n";
	 }

 
 private:
 // Guild-specific data: human-readable name and a set of scoring rule descriptions.
 // You may later replace scoring rules with a structured rule object or callback
 // that applies points to a Player; strings are a simple first step.
 std::string m_guildName;
 Age m_age;
 std::vector<std::string> m_scoringRules;
 };
}
