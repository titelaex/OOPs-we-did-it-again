module Models.Token;

import <fstream>;
import <sstream>;
import <stdexcept>;
import <ostream>;

using namespace Models;

Token::Token(TokenType type, std::string name, std::string description, std::tuple<uint8_t,uint8_t,uint8_t> coins, uint8_t victoryPoints, uint8_t shieldPoints)
	: m_type(type), m_name(std::move(name)), m_description(std::move(description)), m_coins(coins), m_victoryPoints(victoryPoints), m_shieldPoints(shieldPoints) {}

TokenType Token::getType() const noexcept { return m_type; }
const std::string& Token::getName() const noexcept { return m_name; }
const std::string& Token::getDescription() const noexcept { return m_description; }
const std::tuple<uint8_t,uint8_t,uint8_t>& Token::getCoins() const noexcept { return m_coins; }
uint8_t Token::getVictoryPoints() const noexcept { return m_victoryPoints; }
uint8_t Token::getShieldPoints() const noexcept { return m_shieldPoints; }

static std::tuple<uint8_t,uint8_t,uint8_t> ParseCoinsField(const std::string& s) {
	if (s.empty()) return {0,0,0};
	std::istringstream ss(s);
	std::string a,b,c;
	if (!std::getline(ss,a,':')) return {0,0,0};
	if (!std::getline(ss,b,':')) return {static_cast<uint8_t>(std::stoi(a)),0,0};
	if (!std::getline(ss,c)) return {static_cast<uint8_t>(std::stoi(a)), static_cast<uint8_t>(std::stoi(b)),0};
	return {static_cast<uint8_t>(std::stoi(a)), static_cast<uint8_t>(std::stoi(b)), static_cast<uint8_t>(std::stoi(c))};
}

std::vector<Token> Models::createDefaultTokenSet()
{
	return {
		Token(TokenType::PROGRESS, "Agriculture", "Gain extra resources", {0,0,1},0,0),
		Token(TokenType::PROGRESS, "Architecture", "Cheaper wonders", {0,1,0},0,0),
		Token(TokenType::MILITARY, "Shield", "Advance conflict pawn", {0,0,0},0,1)
	};
}

std::vector<Token> Models::loadTokensFromCSV(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) throw std::runtime_error("Unable to open tokens CSV file: " + path);
	std::string header; if (!std::getline(ifs, header)) throw std::runtime_error("Empty tokens CSV file: " + path);
	std::vector<Token> tokens; std::string line;
	while (std::getline(ifs, line)) {
		if (line.empty()) continue; std::istringstream ss(line);
		std::string typeStr, name, desc, coinsField, victoryStr, shieldStr;
		if (!std::getline(ss, typeStr, ',')) continue;
		if (!std::getline(ss, name, ',')) continue;
		if (ss.peek() == '"') { char ch; ss.get(ch); std::getline(ss, desc, '"'); if (ss.peek() == ',') ss.get(ch); }
		else { if (!std::getline(ss, desc, ',')) desc.clear(); }
		if (!std::getline(ss, coinsField, ',')) coinsField.clear();
		if (!std::getline(ss, victoryStr, ',')) victoryStr.clear();
		if (!std::getline(ss, shieldStr)) shieldStr.clear();
		TokenType type = tokenTypeFromString(typeStr);
		auto coins = ParseCoinsField(coinsField);
		uint8_t victory = static_cast<uint8_t>(std::stoi(victoryStr.empty()?"0":victoryStr));
		uint8_t shield = static_cast<uint8_t>(std::stoi(shieldStr.empty()?"0":shieldStr));
		tokens.emplace_back(type, name, desc, coins, victory, shield);
	}
	return tokens;
}

std::ostream& Models::operator<<(std::ostream& os, const Token& t)
{
	std::string typeStr = "UNKNOWN";
	switch (t.getType()) {
		case TokenType::PROGRESS: typeStr = "PROGRESS"; break;
		case TokenType::MILITARY: typeStr = "MILITARY"; break;
	}
	auto [ones, threes, sixes] = t.getCoins();
	uint32_t totalCoins = ones + threes * 3 + sixes * 6;

	os << "Name=" << t.getName()
		<< ";Type=" << typeStr	
		<< ";Description=" << t.getDescription()
		<< ";Coins=" << totalCoins
		<< ";VictoryPoints=" << static_cast<int>(t.getVictoryPoints())
		<< ";ShieldPoints=" << static_cast<int>(t.getShieldPoints());
	return os;
}
