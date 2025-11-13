export module Token;

import <string>;
import <vector>;
import <cstdint>;

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

    export class __declspec(dllexport) Token
    {
    public:
        Token() = delete;
        explicit Token(TokenType type, std::string id, std::string name, std::string description, uint8_t value = 0)
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
        uint8_t m_value = 0;
    };

    export inline __declspec(dllexport) std::vector<Token> CreateDefaultTokenSet()
    {
        std::vector<Token> set;
        set.emplace_back(TokenType::PROGRESS, "progress_architecture", "Architecture", "Reduce structure cost", 0);
        set.emplace_back(TokenType::PROGRESS, "progress_law", "Law", "Reduce science cost", 0);
        set.emplace_back(TokenType::PROGRESS, "progress_strategy", "Strategy", "Military bonus", 0);
        set.emplace_back(TokenType::VICTORY, "victory_3", "Victory +3", "Gives 3 victory points", 3);
        set.emplace_back(TokenType::VICTORY, "victory_5", "Victory +5", "Gives 5 victory points", 5);
        set.emplace_back(TokenType::COIN, "coin_6", "Coin +6", "Gives 6 coins", 6);
        return set;
    }
}