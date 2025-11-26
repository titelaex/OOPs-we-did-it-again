export module Models.Token;

import <string>;
import <vector>;
import <cstdint>;
import <fstream>;
import <sstream>;
import <stdexcept>;
import <tuple>;
import <ostream>; // for operator<< declaration

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
    private:
        TokenType m_type;
        std::string m_name;
        std::string m_description;
        std::tuple<uint8_t,uint8_t,uint8_t> m_coins;
        uint8_t m_victoryPoints{};
        uint8_t m_shieldPoints{};

    public:
        Token() = delete;
        // Construct with explicit coin tuple, victory and shield values
        explicit Token(TokenType type, std::string name, std::string description, std::tuple<uint8_t,uint8_t,uint8_t> coins = {0,0,0}, uint8_t victoryPoints =0, uint8_t shieldPoints =0);

        TokenType getType() const noexcept;
        const std::string& getName() const noexcept;
        const std::string& getDescription() const noexcept;
        const std::tuple<uint8_t,uint8_t,uint8_t>& getCoins() const noexcept;
        uint8_t getVictoryPoints() const noexcept;
        uint8_t getShieldPoints() const noexcept;

    };

    // Returns a small built-in default set. Useful when CSV is unavailable.
    export __declspec(dllexport) std::vector<Token> createDefaultTokenSet();

    // Helper: map uppercase type string to TokenType; throws on unknown type.
    inline TokenType tokenTypeFromString(const std::string& s)
    {
        if (s == "PROGRESS") return TokenType::PROGRESS;
        if (s == "VICTORY") return TokenType::VICTORY;
        if (s == "COIN") return TokenType::COIN;
        if (s == "MILITARY") return TokenType::MILITARY;
        if (s == "SCIENCE") return TokenType::SCIENCE;
        throw std::invalid_argument("Unknown token type: " + s);
    }

    // Load tokens from a CSV file. Expected header: type,name,description,coins,victory,shield
    // - Handles quoted description fields (basic support: "..." without embedded quotes).
    // - On parse error throws std::runtime_error or std::invalid_argument.
    export __declspec(dllexport) std::vector<Token> loadTokensFromCSV(const std::string& path);

    // Stream insertion operator for easy printing
    export __declspec(dllexport) std::ostream& operator<<(std::ostream& os, const Token& t);
}