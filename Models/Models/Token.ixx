export module Models.Token;

import <string>;
import <vector>;
import <cstdint>;
import <fstream>;
import <sstream>;
import <stdexcept>;

export namespace Models
{
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
        explicit Token(TokenType type, std::string id, std::string name, std::string description, uint8_t value =0);

        TokenType GetType() const noexcept;
        const std::string& GetId() const noexcept;
        const std::string& GetName() const noexcept;
        const std::string& GetDescription() const noexcept;
        uint8_t GetValue() const noexcept;

    private:
        TokenType m_type;
        std::string m_id;
        std::string m_name;
        std::string m_description;
        uint8_t m_value =0;
    };

    // Returns a small built-in default set. Useful when CSV is unavailable.
    export __declspec(dllexport) std::vector<Token> CreateDefaultTokenSet();

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
    export __declspec(dllexport) std::vector<Token> LoadTokensFromCSV(const std::string& path);
}