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
        MILITARY,
    };

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
        explicit Token(TokenType type, std::string name, std::string description, std::tuple<uint8_t,uint8_t,uint8_t> coins = {0,0,0}, uint8_t victoryPoints =0, uint8_t shieldPoints =0);

        TokenType getType() const noexcept;
        const std::string& getName() const noexcept;
        const std::string& getDescription() const noexcept;
        const std::tuple<uint8_t,uint8_t,uint8_t>& getCoins() const noexcept;
        uint8_t getVictoryPoints() const noexcept;
        uint8_t getShieldPoints() const noexcept;

    };

    export __declspec(dllexport) std::vector<Token> createDefaultTokenSet();

    inline TokenType tokenTypeFromString(const std::string& s)
    {
        if (s == "PROGRESS") return TokenType::PROGRESS;
        if (s == "MILITARY") return TokenType::MILITARY;
        throw std::invalid_argument("Unknown token type: " + s);
    }
    export __declspec(dllexport) std::vector<Token> loadTokensFromCSV(const std::string& path);
    export __declspec(dllexport) std::ostream& operator<<(std::ostream& os, const Token& t);
}