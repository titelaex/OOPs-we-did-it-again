module Models.Token;
import Models.Token;
import <fstream>;
import <sstream>;
import <stdexcept>;

using namespace Models;

Token::Token(TokenType type, std::string id, std::string name, std::string description, uint8_t value)
	: m_type(type), m_id(std::move(id)), m_name(std::move(name)), m_description(std::move(description)), m_value(value) {}

TokenType Token::GetType() const noexcept { return m_type; }
const std::string& Token::GetId() const noexcept { return m_id; }
const std::string& Token::GetName() const noexcept { return m_name; }
const std::string& Token::GetDescription() const noexcept { return m_description; }
uint8_t Token::GetValue() const noexcept { return m_value; }

std::vector<Token> Models::CreateDefaultTokenSet()
{
	return {
		Token(TokenType::PROGRESS, "P1", "Agriculture", "Gain extra resources", 1),
		Token(TokenType::PROGRESS, "P2", "Architecture", "Cheaper wonders", 2),
		Token(TokenType::MILITARY, "M1", "Shield", "Advance conflict pawn", 1)
	};
}

std::vector<Token> Models::LoadTokensFromCSV(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) throw std::runtime_error("Unable to open tokens CSV file: " + path);
	std::string header; if (!std::getline(ifs, header)) throw std::runtime_error("Empty tokens CSV file: " + path);
	std::vector<Token> tokens; std::string line;
	while (std::getline(ifs, line)) {
		if (line.empty()) continue; std::istringstream ss(line);
		std::string id, typeStr, name, desc, valueStr;
		if (!std::getline(ss, id, ',')) continue;
		if (!std::getline(ss, typeStr, ',')) continue;
		if (!std::getline(ss, name, ',')) continue;
		if (ss.peek() == '"') { char ch; ss.get(ch); std::getline(ss, desc, '"'); if (ss.peek() == ',') ss.get(ch); }
		else { if (!std::getline(ss, desc, ',')) desc.clear(); }
		if (!std::getline(ss, valueStr)) valueStr.clear();
		uint8_t value = static_cast<uint8_t>(std::stoi(valueStr.empty()?"0":valueStr));
		TokenType type = TokenTypeFromString(typeStr); tokens.emplace_back(type, id, name, desc, value);
	}
	return tokens;
}
