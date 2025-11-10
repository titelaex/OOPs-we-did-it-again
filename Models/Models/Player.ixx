export module Player;

import Wonder;
import Token;
import ResourceType;

import Card; // de transformat in modul

import <string>;
import <vector>;
import <tuple>;
import <cstdint>;
import <unordered_map>;

namespace Models
{
	export class Player
	{
	private:
		const uint8_t kplayerId; //login info / to track turns
		const std::string kplayerUsername; //login info

		uint8_t positionOnBoard; // will be 0 in the constructor

		std::vector<Wonder> m_ownedWonders; //Wonders needs to be its own class that inherits from Cards
		std::vector<Card> m_ownedCards; ///age cards + guilds
		std::vector<Token> m_ownedTokens; //military tokens + progress tokens
		bool m_hasConflictPawn = false;


		std::unordered_map<ResourceType, uint8_t> m_ownedPermanentResources; //key: resource type, value: quantity
		//toate resursele temporare, adica cele care provin din alegeri (spre ex. din cartile galbene) in fiecare runda
		std::unordered_map<ResourceType, uint8_t> m_ownedTradingResources; //key: resource type, value: quantity produced per turn

		std::tuple<uint8_t, uint8_t, uint8_t> m_remainingCoins; // 1 x coins of 1, 2 x coins of 3, 0 x coins of 6

		struct Points {
			uint8_t m_militaryVictoryPoints;
			uint8_t m_buildingVictoryPoints;
			uint8_t m_wonderVictoryPoints;
			uint8_t progressVictoryPoints;

			uint8_t totalVictoryPoints() {
				return m_militaryVictoryPoints + m_buildingVictoryPoints + m_wonderVictoryPoints + progressVictoryPoints;
			}
		} m_playerPoints;


	public:

		/// implementation of the constructor
		Player(const uint8_t& id, const std::string& username) : kplayerId(id), kplayerUsername(username)
		{
			positionOnBoard = 0;
			m_remainingCoins = std::make_tuple(1, 2, 0); // 3 coins of 1, 0 coins of 3, 0 coins of 6
			m_playerPoints = { 0,0,0,0 };
		}

		~Player() = default;


		uint8_t totalCoins(std::tuple<uint8_t, uint8_t, uint8_t> coins)
		{
			return std::get<0>(coins) + std::get<1>(coins) * 3 + std::get<2>(coins) * 6;
		}


		//implementation of setters
		void setPositionOnBoard(uint8_t position)
		{
			positionOnBoard = position;
		}

		void addCard(const Card& card)
		{
			m_ownedCards.push_back(card);
		}

		void addWonder(const Wonder& wonder)
		{
			m_ownedWonders.push_back(wonder);
		}
		void addToken(const Token& token)
		{
			m_ownedTokens.push_back(token);
		}

		void addPermanentResource(ResourceType resourceType, uint8_t quantity)
		{
			m_ownedPermanentResources[resourceType] += quantity;
		}
		void addTradingResource(ResourceType resourceType, uint8_t quantity)
		{
			m_ownedTradingResources[resourceType] += quantity;
		}

		void setHasConflictPawn(bool hasPawn)
		{
			m_hasConflictPawn = hasPawn;
		}

		void setRemainingCoins(std::tuple<uint8_t, uint8_t, uint8_t> remainingCoins)
		{
			m_remainingCoins = remainingCoins;
		}

		void setPoints(Points playerPoints)
		{
			m_playerPoints = playerPoints;
		}

		//implementation of getters
		const uint8_t getkPlayerId() const
		{
			return kplayerId;
		}

		const std::string& getPlayerUsername() const
		{
			return kplayerUsername;
		}

		int getPositionOnBoard() const
		{
			return positionOnBoard;
		}

		const std::vector<Wonder>& getOwnedWonders() const
		{
			return m_ownedWonders;
		}

		const std::vector<Card>& getOwnedCards() const
		{
			return m_ownedCards;
		}

		const std::vector<Token>& getOwnedTokens() const
		{
			return m_ownedTokens;
		}

		const std::unordered_map<ResourceType, uint8_t>& getOwnedPermanentResources() const
		{
			return m_ownedPermanentResources;
		}

		const std::unordered_map<ResourceType, uint8_t>& getOwnedTradingResources() const
		{
			return m_ownedTradingResources;
		}

		bool getHasConflictPawn() const
		{
			return m_hasConflictPawn;
		}

		std::tuple<uint8_t, uint8_t, uint8_t> getRemainingCoins() const
		{
			return m_remainingCoins;
		}

		Points getPoints() const
		{
			return m_playerPoints;
		}

		void addCoins(uint8_t coinsToAdd)
		{
			auto& [ones, threes, sixes] = m_remainingCoins;

			while (coinsToAdd >= 6)
			{
				++sixes;
				coinsToAdd -= 6;
			}
			while (coinsToAdd >= 3)
			{
				++threes;
				coinsToAdd -= 3;
			}
			while (coinsToAdd > 0)
			{
				++ones;
				--coinsToAdd;
			}
		}

		
		uint8_t countYellowCards() const
		{
			uint8_t count = 0;
			for (const auto& card : m_ownedCards)
			{
				if (card.GetColor() == ColorType::YELLOW)
					++count;
			}
			return count;
		}


	};
}