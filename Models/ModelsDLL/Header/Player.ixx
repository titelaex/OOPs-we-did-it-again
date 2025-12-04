export module Models.Player;

import <string>;
import <vector>;
import <tuple>;
import <cstdint>;
import <unordered_map>;
import <memory>;
import Models.Token;
import Models.ResourceType;
import Models.Card; 
import Models.Wonder; 
import Models.ScientificSymbolType;
import Models.TradeRuleType;

namespace Models
{
	export class __declspec(dllexport) Player
	{
	private:
		const uint8_t kplayerId;
		std::string m_playerUsername;
		std::vector<std::unique_ptr<Wonder>> m_ownedWonders;
		std::vector<std::unique_ptr<Card>> m_ownedCards;
		std::vector<Token> m_ownedTokens;
		std::unordered_map<ResourceType, uint8_t> m_ownedPermanentResources;
		std::unordered_map<ResourceType, uint8_t> m_ownedTradingResources;
		std::unordered_map<ScientificSymbolType, uint8_t> m_ownedScientificSymbols;
		std::tuple<uint8_t, uint8_t, uint8_t> m_remainingCoins{ 1,2,0 };
		struct Points {
			uint8_t m_militaryVictoryPoints;
			uint8_t m_buildingVictoryPoints;
			uint8_t m_wonderVictoryPoints;
			uint8_t m_progressVictoryPoints;
			uint8_t totalVictoryPoints();
		} m_playerPoints;
		std::unordered_map<TradeRuleType, bool> m_tradeRules{ 0 };
	public:
		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;
		Player(Player&&) noexcept = default;
		Player& operator=(Player&&) noexcept = default;
		Player(const uint8_t& id, const std::string& username);
		~Player() = default;
		uint8_t totalCoins(const std::tuple<uint8_t, uint8_t, uint8_t>& coins);
		void addCard(const std::unique_ptr<Card>& card);
		void addWonder(const std::unique_ptr<Wonder>& wonder);
		void addToken(const Token& token);
		void addPermanentResource(const ResourceType& resourceType, const uint8_t& quantity);
		void addTradingResource(const ResourceType& resourceType, const uint8_t& quantity);
		void setRemainingCoins(const std::tuple<uint8_t, uint8_t, uint8_t>& remainingCoins);
		void setPoints(const Points& playerPoints);
		void addScientificSymbol(const ScientificSymbolType& symbol, const uint8_t& quantity);
		void setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		const uint8_t getkPlayerId() const;
		const std::string& getPlayerUsername() const;
		const std::vector<std::unique_ptr<Wonder>>& getOwnedWonders() const;
		const std::vector<std::unique_ptr<Card>>& getOwnedCards() const;
		const std::vector<Token>& getOwnedTokens() const;
		const std::unordered_map<ResourceType, uint8_t>& getOwnedPermanentResources() const;
		const std::unordered_map<ResourceType, uint8_t>& getOwnedTradingResources() const;
		const std::tuple<uint8_t, uint8_t, uint8_t> getRemainingCoins() const;
		const std::unordered_map<ScientificSymbolType, uint8_t>& getOwnedScientificSymbols() const;
		const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		const Points& getPoints() const;
	};
}