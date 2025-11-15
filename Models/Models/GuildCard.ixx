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
import <fstream>;
import <sstream>;
import <stdexcept>;

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

 // Load guild cards from a CSV file. Expected header: guild_name,scoring_rules,description
 // scoring_rules are semicolon-separated; description is optional and ignored by loader currently.
 export inline std::vector<GuildCard> LoadGuildCardsFromCSV(const std::string& path)
 {
 std::ifstream ifs(path);
 if (!ifs.is_open())
 throw std::runtime_error("Unable to open guild CSV file: " + path);

 std::string header;
 if (!std::getline(ifs, header))
 throw std::runtime_error("Empty guild CSV file: " + path);

 std::vector<GuildCard> cards;
 std::string line;
 while (std::getline(ifs, line))
 {
 if (line.empty()) continue;
 std::istringstream ss(line);
 std::string name, rulesField, desc;

 // parse name
 if (!std::getline(ss, name, ',')) continue;

 // parse rules (may be quoted and contain commas/semicolons)
 if (ss.peek() == '"')
 {
 char ch; ss.get(ch); // consume '"'
 std::getline(ss, rulesField, '"');
 if (ss.peek() == ',') ss.get(ch);
 }
 else
 {
 if (!std::getline(ss, rulesField, ',')) rulesField.clear();
 }

 // remaining is description (ignored)
 if (!std::getline(ss, desc)) desc.clear();

 // trim helper
 auto trim = [](std::string& s){
 size_t a = s.find_first_not_of(" \t\r\n");
 size_t b = s.find_last_not_of(" \t\r\n");
 if (a == std::string::npos) { s.clear(); return; }
 s = s.substr(a, b - a +1);
 };
 trim(name);
 trim(rulesField);

 // split rulesField by ';' into vector<string>
 std::vector<std::string> rules;
 std::istringstream rs(rulesField);
 std::string token;
 while (std::getline(rs, token, ';'))
 {
 trim(token);
 if (!token.empty()) rules.push_back(token);
 }

 if (name.empty()) continue;
 cards.emplace_back(name, rules);
 }

 return cards;
 }
}
