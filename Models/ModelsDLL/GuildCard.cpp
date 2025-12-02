#include "../../Core/CardCsvParser.cpp"
//module Models.GuildCard;
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
	const std::string& modelPath)
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
	) {}

//// CSV/minimal constructor: provide sensible defaults for base Card
//GuildCard::GuildCard(const std::string& name, const std::vector<std::string>& scoringRules)
//	: Card(name, std::unordered_map<ResourceType, uint8_t>{},0, CoinWorthType::VALUE,0, std::string{}, ColorType::BROWN, false, std::string{}),
//	m_guildName(name), m_scoringRules(scoringRules)
//{
//}

GuildCard::GuildCard(GuildCard&& other) noexcept
	: Card(std::move(other)) {}
GuildCard& GuildCard::operator=(GuildCard&& other) noexcept { if (this != &other) { Card::operator=(std::move(other));  } return *this; }

void GuildCard::toggleVisibility() { Card::toggleVisibility(); }
void GuildCard::toggleAccessibility() { Card::toggleAccessibility(); }

void GuildCard::displayCardInfo() {
	Card::displayCardInfo();
	std::cout << "Guild Name: " << Card::GetName() << "\n";
	auto type = Card::GetCoinWorth();
	switch (type)
	{
		case CoinWorthType::WONDER:
			std::cout << " 2 victory points for each Wonder constructed in the city which has the most wonders.";
			break;
		case CoinWorthType::BLUE:
			std::cout << "  1 coin/blue card in the city which has the most blue cards at that time + 1v_point/blue card ath the end of the game";
			break;
		case CoinWorthType::GREYBROWN:
			std::cout << "1 coin/(gray+brown in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		case CoinWorthType::RED:
			std::cout << "1 coin/(red in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		case CoinWorthType::YELLOW:
			std::cout << "1 coin/(yellow in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		case CoinWorthType::VALUE:
			std::cout << " at the end h 1 victory point for each set of 3 coins in the richest city.";
			break;
		case CoinWorthType::GREEN:
			std::cout << "1 coin/(green in the city which has the most at that time)+ 1v_point/(same thing at the end)";
			break;
		deafault:
			std::cout << "Tip neidentificat";
	}
}

//std::vector<GuildCard> Models::LoadGuildCardsFromCSV(const std::string& path)
//{
//	std::ifstream ifs(path);
//	if (!ifs.is_open()) throw std::runtime_error("Unable to open guild CSV file: " + path);
//	std::string header; if (!std::getline(ifs, header)) throw std::runtime_error("Empty guild CSV file: " + path);
//	std::vector<GuildCard> cards; std::string line;
//	while (std::getline(ifs, line)) {
//		if (line.empty()) continue; std::istringstream ss(line);
//		std::string name, rulesField, desc;
//		if (!std::getline(ss, name, ',')) continue;
//		if (ss.peek() == '"') { char ch; ss.get(ch); std::getline(ss, rulesField, '"'); if (ss.peek() == ',') ss.get(ch); }
//		else { if (!std::getline(ss, rulesField, ',')) rulesField.clear(); }
//		if (!std::getline(ss, desc)) desc.clear();
//		auto trim = [](std::string& s){ size_t a = s.find_first_not_of(" \t\r\n"); size_t b = s.find_last_not_of(" \t\r\n"); if (a == std::string::npos){ s.clear(); return;} s = s.substr(a, b-a+1);};
//		trim(name); trim(rulesField);
//		std::vector<std::string> rules; std::istringstream rs(rulesField); std::string token; while (std::getline(rs, token, ';')) { trim(token); if (!token.empty()) rules.push_back(token); }
//		if (name.empty()) continue; GuildCard gc(name, rules); cards.push_back(std::move(gc));
//	}
//	return cards;
//}


std::vector<GuildCard> Models::LoadGuildCardsFromCSV(const std::string& path)
{
	// Use the generic CSV parser with the factory implemented in Core\CardCsvParser.cpp
	return ParseCardsFromCSV<GuildCard>(path, GuildCardFactory);
}