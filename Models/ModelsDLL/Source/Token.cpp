module Models.Token;

import <fstream>;
import <sstream>;
import <stdexcept>;
import <ostream>;
import <istream>;
import <vector>;
import <functional>;
import <utility>;
import <tuple>;
import <string>;

using namespace Models;

Token::Token(TokenType type, std::string name, std::string description, std::tuple<uint8_t,uint8_t,uint8_t> coins, uint8_t victoryPoints, uint8_t shieldPoints)
	: m_type(type), m_name(std::move(name)), m_description(std::move(description)), m_coins(coins), m_victoryPoints(victoryPoints), m_shieldPoints(shieldPoints) {}

TokenType Token::getType() const noexcept { return m_type; }
const std::string& Token::getName() const noexcept { return m_name; }
const std::string& Token::getDescription() const noexcept { return m_description; }
const std::tuple<uint8_t,uint8_t,uint8_t>& Token::getCoins() const noexcept { return m_coins; }
uint8_t Token::getVictoryPoints() const noexcept { return m_victoryPoints; }
uint8_t Token::getShieldPoints() const noexcept { return m_shieldPoints; }
const std::vector<std::pair<std::function<void()>, std::string>>& Token::getOnPlayActions() const noexcept { return m_onPlayActions; }
void Token::setOnPlayActions(std::vector<std::pair<std::function<void()>, std::string>> actions) { m_onPlayActions = std::move(actions); }

void Token::setType(TokenType type) { m_type = type; }
void Token::setName(const std::string& name) { m_name = name; }
void Token::setDescription(const std::string& description) { m_description = description; }
void Token::setCoins(const std::tuple<uint8_t,uint8_t,uint8_t>& coins) { m_coins = coins; }
void Token::setVictoryPoints(uint8_t points) { m_victoryPoints = points; }
void Token::setShieldPoints(uint8_t points) { m_shieldPoints = points; }

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

namespace
{
	std::string csvEscape(const std::string& s)
	{
		std::string out;
		out.reserve(s.size() + 2);
		out.push_back('"');
		for (char ch : s) {
			if (ch == '"') out += "\"\"";
			else out.push_back(ch);
		}
		out.push_back('"');
		return out;
	}

	std::string csvUnescape(const std::string& s)
	{
		if (s.empty()) return s;
		std::string result;
		result.reserve(s.size());
		size_t start = 0;
		size_t end = s.size();
		if (s.front() == '"' && s.back() == '"' && s.size() >= 2) {
			start = 1;
			end = s.size() - 1;
		}
		for (size_t i = start; i < end; ++i) {
			if (s[i] == '"' && i + 1 < end && s[i + 1] == '"') {
				result.push_back('"');
				++i;
			} else {
				result.push_back(s[i]);
			}
		}
		return result;
	}

	std::vector<std::string> parseCsvFields(const std::string& line)
	{
		std::vector<std::string> fields;
		std::string field;
		bool in_quotes = false;
		for (size_t i = 0; i < line.size(); ++i) {
			char c = line[i];
			if (c == '"') {
				in_quotes = !in_quotes;
				field.push_back(c);
			} else if (c == ',' && !in_quotes) {
				fields.push_back(field);
				field.clear();
			} else {
				field.push_back(c);
			}
		}
		fields.push_back(field);
		return fields;
	}

	std::string actionPairVectorToString(const std::vector<std::pair<std::function<void()>, std::string>>& actions)
	{
		std::string s;
		bool first = true;
		for (const auto& actionPair : actions) {
			if (!first) s += ", ";
			first = false;
			s += actionPair.second;
		}
		return s;
	}

	std::string coinsTupleToString(const std::tuple<uint8_t,uint8_t,uint8_t>& coins)
	{
		auto [ones, threes, sixes] = coins;
		if (threes == 0 && sixes == 0) {
			if (ones == 0) return "";
			return std::to_string(static_cast<int>(ones));
		}
		return std::to_string(static_cast<int>(ones)) + ':' +
			std::to_string(static_cast<int>(threes)) + ':' +
			std::to_string(static_cast<int>(sixes));
	}

	std::string tokenTypeToString(TokenType type)
	{
		switch (type) {
		case TokenType::PROGRESS: return "PROGRESS";
		case TokenType::MILITARY: return "MILITARY";
		default: return "UNKNOWN";
		}
	}
}

std::ostream& Models::operator<<(std::ostream& os, const Token& t)
{
	os << csvEscape(tokenTypeToString(t.getType())) << ',';
	os << csvEscape(t.getName()) << ',';
	os << csvEscape(t.getDescription()) << ',';
	os << csvEscape(coinsTupleToString(t.getCoins())) << ',';
	os << '"';
	if (t.getVictoryPoints() > 0) {
		os << static_cast<int>(t.getVictoryPoints());
	}
	os << '"' << ',';
	os << '"';
	if (t.getShieldPoints() > 0) {
		os << static_cast<int>(t.getShieldPoints());
	}
	os << '"' << ',';
	os << csvEscape(actionPairVectorToString(t.getOnPlayActions()));

	return os;
}

std::istream& Models::operator>>(std::istream& is, Token& t)
{
	std::string line;
	if (!std::getline(is, line)) {
		return is;
	}

	auto fields = parseCsvFields(line);
	if (fields.size() < 6) {
		is.setstate(std::ios::failbit);
		return is;
	}
	std::string typeStr = csvUnescape(fields[0]);
	try {
		t.setType(tokenTypeFromString(typeStr));
	} catch (...) {
		t.setType(TokenType::PROGRESS);
	}
	t.setName(csvUnescape(fields[1]));
	t.setDescription(csvUnescape(fields[2]));
	std::string coinsStr = csvUnescape(fields[3]);
	t.setCoins(ParseCoinsField(coinsStr));
	std::string victoryStr = csvUnescape(fields[4]);
	if (!victoryStr.empty()) {
		try {
			t.setVictoryPoints(static_cast<uint8_t>(std::stoi(victoryStr)));
		} catch (...) {
			t.setVictoryPoints(0);
		}
	} else {
		t.setVictoryPoints(0);
	}
	std::string shieldStr = csvUnescape(fields[5]);
	if (!shieldStr.empty()) {
		try {
			t.setShieldPoints(static_cast<uint8_t>(std::stoi(shieldStr)));
		} catch (...) {
			t.setShieldPoints(0);
		}
	} else {
		t.setShieldPoints(0);
	}

	return is;
}
