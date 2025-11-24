module Models.GuildCard;
import <fstream>;
import <sstream>;
import <stdexcept>;
import <iostream>;
import Models.GuildCard;

using namespace Models;
GuildCard::GuildCard(const std::string& name,
	const std::unordered_map<ResourceType, uint8_t>& resourceCost,
	uint8_t victoryPoints,
	CoinWorthType coinWorth,
	uint8_t coinReward,
	const std::string& caption,
	ColorType color,
	bool isVisible,
	const std::string& modelPath,
	const std::vector<std::string>& scoringRules)
	: Card(
		name,
		 resourceCost,
		victoryPoints,
		coinWorth,
		coinReward,
		caption,
		color,
		isVisible,
		modelPath
	), m_scoringRules(scoringRules) {
}

// CSV/minimal constructor: provide sensible defaults for base Card
GuildCard::GuildCard(const std::string& name, const std::vector<std::string>& scoringRules)
	: Card(name, std::unordered_map<ResourceType, uint8_t>{},0, CoinWorthType::VALUE,0, std::string{}, ColorType::BROWN, false, std::string{}),
	m_guildName(name), m_scoringRules(scoringRules)
{
}

GuildCard::GuildCard(GuildCard&& other) noexcept
	: Card(std::move(other)), m_guildName(std::move(other.m_guildName)), m_scoringRules(std::move(other.m_scoringRules)) {}
GuildCard& GuildCard::operator=(GuildCard&& other) noexcept { if (this != &other) { Card::operator=(std::move(other)); m_guildName = std::move(other.m_guildName); m_scoringRules = std::move(other.m_scoringRules); } return *this; }

const std::string& GuildCard::GetGuildName() const noexcept { return m_guildName; }
const std::vector<std::string>& GuildCard::GetScoringRules() const noexcept { return m_scoringRules; }
void GuildCard::SetGuildName(const std::string& name) { m_guildName = name; }
void GuildCard::SetScoringRules(const std::vector<std::string>& rules) { m_scoringRules = rules; }

void GuildCard::toggleVisibility() { Card::toggleVisibility(); }
void GuildCard::toggleAccessibility() { Card::toggleAccessibility(); }

void GuildCard::displayCardInfo() {
	Card::displayCardInfo();
	std::cout << "Guild Name: " << m_guildName << "\n";
	std::cout << "Scoring Rules:\n";
	for (const auto& r : m_scoringRules) std::cout << " - " << r << "\n";
}

std::vector<GuildCard> Models::LoadGuildCardsFromCSV(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) throw std::runtime_error("Unable to open guild CSV file: " + path);
	std::string header; if (!std::getline(ifs, header)) throw std::runtime_error("Empty guild CSV file: " + path);
	std::vector<GuildCard> cards; std::string line;
	while (std::getline(ifs, line)) {
		if (line.empty()) continue; std::istringstream ss(line);
		std::string name, rulesField, desc;
		if (!std::getline(ss, name, ',')) continue;
		if (ss.peek() == '"') { char ch; ss.get(ch); std::getline(ss, rulesField, '"'); if (ss.peek() == ',') ss.get(ch); }
		else { if (!std::getline(ss, rulesField, ',')) rulesField.clear(); }
		if (!std::getline(ss, desc)) desc.clear();
		auto trim = [](std::string& s){ size_t a = s.find_first_not_of(" \t\r\n"); size_t b = s.find_last_not_of(" \t\r\n"); if (a == std::string::npos){ s.clear(); return;} s = s.substr(a, b-a+1);};
		trim(name); trim(rulesField);
		std::vector<std::string> rules; std::istringstream rs(rulesField); std::string token; while (std::getline(rs, token, ';')) { trim(token); if (!token.empty()) rules.push_back(token); }
		if (name.empty()) continue; GuildCard gc(name, rules); cards.push_back(gc);
	}
	return cards;
}
