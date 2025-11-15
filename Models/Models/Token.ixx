export module Token;

import <string>;
import <vector>;
import <cstdint>;
import <fstream>;
import <sstream>;
import <stdexcept>;

export namespace Models
{
    // TokenType describes the kind of token. __declspec(dllexport) kept for DLL boundary.
    export enum class __declspec(dllexport) TokenType : uint8_t
    {
        PROGRESS,
        VICTORY,
        COIN,
        MILITARY,
        SCIENCE
    };

    // Simple Token value object. No default ctor to force explicit initialization.
    export class __declspec(dllexport) Token
    {
    public:
        Token() = delete;
        explicit Token(TokenType type, std::string id, std::string name, std::string description, uint8_t value =0)
            : m_type(type), m_id(std::move(id)), m_name(std::move(name)), m_description(std::move(description)), m_value(value) {
        }

        TokenType GetType() const noexcept { return m_type; }
        const std::string& GetId() const noexcept { return m_id; }
        const std::string& GetName() const noexcept { return m_name; }
        const std::string& GetDescription() const noexcept { return m_description; }
        uint8_t GetValue() const noexcept { return m_value; }

    private:
        TokenType m_type;
        std::string m_id;
        std::string m_name;
        std::string m_description;
        uint8_t m_value =0;
    };

    // Returns a small built-in default set. Useful when CSV is unavailable.
    export inline __declspec(dllexport) std::vector<Token> CreateDefaultTokenSet()
    {
        std::vector<Token> set;
        set.emplace_back(TokenType::PROGRESS, "progress_architecture", "Architecture", "Reduce structure cost",0);
        set.emplace_back(TokenType::PROGRESS, "progress_law", "Law", "Reduce science cost",0);
        set.emplace_back(TokenType::PROGRESS, "progress_strategy", "Strategy", "Military bonus",0);
        set.emplace_back(TokenType::VICTORY, "victory_3", "Victory +3", "Gives3 victory points",3);
        set.emplace_back(TokenType::VICTORY, "victory_5", "Victory +5", "Gives5 victory points",5);
        set.emplace_back(TokenType::COIN, "coin_6", "Coin +6", "Gives6 coins",6);
        return set;
    }

    // Helper: map uppercase type string to TokenType; throws on unknown type.
    inline TokenType TokenTypeFromString(const std::string& s)
    {
        if (s == "PROGRESS") return TokenType::PROGRESS;
        if (s == "VICTORY") return TokenType::VICTORY;
        if (s == "COIN") return TokenType::COIN;
        if (s == "MILITARY") return TokenType::MILITARY;
        if (s == "SCIENCE") return TokenType::SCIENCE;
        throw std::invalid_argument("Unknown token type: " + s);
    }

    // Load tokens from a CSV file. Expected header: id,type,name,description,value
    // - Handles quoted description fields (basic support: "..." without embedded quotes).
    // - On parse error throws std::runtime_error or std::invalid_argument.
    export inline __declspec(dllexport) std::vector<Token> LoadTokensFromCSV(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
            throw std::runtime_error("Unable to open token CSV file: " + path);

        std::string header;
        if (!std::getline(ifs, header))
            throw std::runtime_error("Empty token CSV file: " + path);

        std::vector<Token> tokens;
        std::string line;
        while (std::getline(ifs, line))
        {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string id, typeStr, name, description, valueStr;

            // parse id,type,name
            if (!std::getline(ss, id, ',')) continue;
            if (!std::getline(ss, typeStr, ',')) continue;
            if (!std::getline(ss, name, ',')) continue;

            // description may be quoted and contain commas
            if (ss.peek() == '"')
            {
                // consume opening '"'
                char ch; ss.get(ch);
                std::getline(ss, description, '"');
                // consume trailing comma if present
                if (ss.peek() == ',') ss.get(ch);
            }
            else
            {
                if (!std::getline(ss, description, ',')) description.clear();
            }

            // remaining is value (read until comma/newline)
            if (!std::getline(ss, valueStr)) valueStr.clear();

            // trim possible whitespace from fields (simple)
            auto trim = [](std::string& s){
                size_t a = s.find_first_not_of(" \t\r\n");
                size_t b = s.find_last_not_of(" \t\r\n");
                if (a == std::string::npos) { s.clear(); return; }
                s = s.substr(a, b - a +1);
            };
            trim(id); trim(typeStr); trim(name); trim(description); trim(valueStr);

            // parse value
            int v =0;
            if (!valueStr.empty())
            {
                try { v = std::stoi(valueStr); }
                catch (...) { v =0; }
            }

            // map type string to TokenType (allow uppercase in CSV)
            TokenType tt = TokenType::PROGRESS;
            try { tt = TokenTypeFromString(typeStr); }
            catch (const std::invalid_argument&)
            {
                // unknown type -> skip this token with a conservative approach
                continue;
            }

            tokens.emplace_back(tt, id, name, description, static_cast<uint8_t>(v));
        }

        return tokens;
    }
}