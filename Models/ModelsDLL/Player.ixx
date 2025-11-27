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

namespace Models
{
	export class __declspec(dllexport) Player
	{
	private:
		const uint8_t kplayerId;
		const std::string kplayerUsername;
		std::vector<std::unique_ptr<Wonder>> m_ownedWonders;
		std::vector<std::unique_ptr<Card>> m_ownedCards;
		std::vector<Token> m_ownedTokens;
		bool m_hasConflictPawn = false;
		std::unordered_map<ResourceType, uint8_t> m_ownedPermanentResources;
		std::unordered_map<ResourceType, uint8_t> m_ownedTradingResources;
		std::tuple<uint8_t, uint8_t, uint8_t> m_remainingCoins{ 1,2,0 };
		struct Points {
			uint8_t m_militaryVictoryPoints;
			uint8_t m_buildingVictoryPoints;
			uint8_t m_wonderVictoryPoints;
			uint8_t m_progressVictoryPoints;
			uint8_t totalVictoryPoints();
		} m_playerPoints;
	public:
		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;
		Player(Player&&) noexcept = default;
		Player& operator=(Player&&) noexcept = default;
		Player(const uint8_t& id, const std::string& username);
		~Player() = default;
		uint8_t totalCoins(std::tuple<uint8_t, uint8_t, uint8_t> coins);
		void addCard(std::unique_ptr<Card> card);
		void addWonder(std::unique_ptr<Wonder> wonder);
		void addToken(const Token& token);
		void addPermanentResource(ResourceType resourceType, uint8_t quantity);
		void addTradingResource(ResourceType resourceType, uint8_t quantity);
		void setHasConflictPawn(bool hasPawn);
		void setRemainingCoins(std::tuple<uint8_t, uint8_t, uint8_t> remainingCoins);
		void setPoints(Points playerPoints);
		const uint8_t getkPlayerId() const;
		const std::string& getPlayerUsername() const;
		const std::vector<std::unique_ptr<Wonder>>& getOwnedWonders() const;
		const std::vector<std::unique_ptr<Card>>& getOwnedCards() const;
		const std::vector<Token>& getOwnedTokens() const;
		const std::unordered_map<ResourceType, uint8_t>& getOwnedPermanentResources() const;
		const std::unordered_map<ResourceType, uint8_t>& getOwnedTradingResources() const;
		bool getHasConflictPawn() const;
		std::tuple<uint8_t, uint8_t, uint8_t> getRemainingCoins() const;
		Points getPoints() const;
		/*void addCoins(uint8_t coinsToAdd);
		void subtractCoins(uint8_t coinsToSubtract);
		uint8_t countYellowCards() const;
		void burnCard(Card & card);*/
	};
}