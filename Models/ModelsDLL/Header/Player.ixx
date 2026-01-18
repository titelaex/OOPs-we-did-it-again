export module Models.Player;

import <bitset>;
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
	export enum class __declspec(dllexport) TokenIndex : uint8_t {
		AGRICULTURE = 0,
		ARCHITECTURE = 1,
		ECONOMY = 2,
		LAW = 3,
		MASONRY = 4,
		MATHEMATICS = 5,
		PHILOSOPHY = 6,
		STRATEGY = 7,
		THEOLOGY = 8,
		URBANISM = 9
	};

	export class __declspec(dllexport) Player
	{
	private:
		uint8_t playerId;
		std::string m_playerUsername;
		std::vector<std::unique_ptr<Wonder>> m_ownedWonders;
		std::vector<std::unique_ptr<Card>> m_ownedCards;
		std::vector<std::unique_ptr<Token>> m_ownedTokens;
		std::unordered_map<ResourceType, uint8_t> m_ownedPermanentResources;
		std::unordered_map<ResourceType, uint8_t> m_ownedTradingResources;
		std::unordered_map<ScientificSymbolType, uint8_t> m_ownedScientificSymbols;
		std::tuple<uint8_t, uint8_t, uint8_t> m_remainingCoins{ 1,2,0 };
		struct Points {
			uint8_t m_militaryVictoryPoints;
			uint8_t m_buildingVictoryPoints;
			uint8_t m_wonderVictoryPoints;
			uint8_t m_progressVictoryPoints;
			uint8_t m_coinVictoryPoints;
			__declspec(dllexport) uint8_t totalVictoryPoints() const;
		} m_playerPoints;
		std::unordered_map<TradeRuleType, bool> m_tradeRules{ 0 };
		std::bitset<10> m_tokensOwned{};
		// Agriculture, Architecture, Economy, Law, Mansory, Mathematics, Philosophy, Strategy, Theology, Urbanism

	public:
		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;
		Player(Player&&) noexcept = default;
		Player& operator=(Player&&) noexcept = default;
		Player(const uint8_t& id, const std::string& username);
		Player() = default;
		~Player() = default;
		uint8_t totalCoins(const std::tuple<uint8_t, uint8_t, uint8_t>& coins);
		// replace these three declarations:
		void addCard(std::unique_ptr<Card> card);
		void addWonder(std::unique_ptr<Wonder> wonder);
		void addToken(std::unique_ptr<Token> token);
		void addPermanentResource(const ResourceType& resourceType, const uint8_t& quantity);
		void addTradingResource(const ResourceType& resourceType, const uint8_t& quantity);
		void setPlayerUsername(const std::string& username);
		void setRemainingCoins(const std::tuple<uint8_t, uint8_t, uint8_t>& remainingCoins);
		void setPoints(const Points& playerPoints);
		void addScientificSymbol(const ScientificSymbolType& symbol, const uint8_t& quantity);
		void setTradeRules(const std::unordered_map<TradeRuleType, bool>& tradeRules);
		void setTokensOwned(const std::bitset<10>& tokensOwned);
		const uint8_t getkPlayerId() const;
		const std::string& getPlayerUsername() const;
		std::vector<std::unique_ptr<Models::Wonder>>& getOwnedWonders();
		const std::vector<std::unique_ptr<Card>>& getOwnedCards() const;
		std::unique_ptr<Card> removeOwnedCardAt(size_t idx);
		const std::vector<std::unique_ptr<Token>>& getOwnedTokens() const;
		const std::unordered_map<ResourceType, uint8_t>& getOwnedPermanentResources() const;
		const std::unordered_map<ResourceType, uint8_t>& getOwnedTradingResources() const;
		const std::tuple<uint8_t, uint8_t, uint8_t> getRemainingCoins() const;
		const std::unordered_map<ScientificSymbolType, uint8_t>& getOwnedScientificSymbols() const;
		const std::unordered_map<TradeRuleType, bool>& getTradeRules() const;
		const std::bitset<10>& getTokensOwned() const;
		const Points& getPoints() const;
		uint8_t getTotalVictoryPoints() const;
		
		uint8_t getBlueBuildingVictoryPoints() const;
		
		bool hasToken(TokenIndex tokenIndex) const;
		uint8_t getTokenVictoryBonus() const;
	};

}