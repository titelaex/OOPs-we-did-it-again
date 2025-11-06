module Models.Player;

import Models.Wonder;
#include "Card.h"; //to be transformed into a module

import <string>;
import <vector>;
import <tuple>;
import <cstdint>;

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
//	std::vector<Token> m_ownedTokens; //military tokens + progress tokens
	bool m_hasConflictPawn = false;

	std::tuple<uint8_t, uint8_t, uint8_t> m_remainingCoins; // 1 x coins of 1, 2 x coins of 3, 0 x coins of 6

	struct Points {
		uint8_t m_militaryVictoryPoints;
		uint8_t m_buildindgVictoryPoints;
		uint8_t m_wonderVictoryPoints;
		uint8_t progressVictoryPoints;

		uint8_t totalVictoryPoints() {
			return m_militaryVictoryPoints + m_buildindgVictoryPoints + m_wonderVictoryPoints + progressVictoryPoints;
		}
	} m_playerPoints;


public:

	uint8_t totalCoins(std::tuple<uint8_t, uint8_t, uint8_t> coins);

	//constructor
	Player(const uint8_t& id, const std::string& username) : kplayerId(id), kplayerUsername(username)
	{
		positionOnBoard = 0;
		m_remainingCoins = std::make_tuple(1, 2, 0); // 3 coins of 1, 0 coins of 3, 0 coins of 6
		m_playerPoints = { 0,0,0,0 };
	}
	~Player() = default;

	//setters

	void setPositionOnBoard(uint8_t position);

	void addWonder(const Wonder& wonder);
	void addCard(const Card& card);

	void setHasConflictPawn(bool hasPawn);
	void setRemainingCoins(uint8_t coins);

	void setMilitaryVictoryPoints(uint8_t points);
	void setBuildingVictoryPoints(uint8_t points);
	void setWonderVictoryPoints(uint8_t points);
	void setProgressVictoryPoints(uint8_t points);

	//getters
	const std::string& getPlayerId() const;
	const std::string& getPlayerUsername() const;
	const std::string& getPassword() const;

	int getPositionOnBoard() const;
	const std::vector<Wonder>& getOwnedWonders() const;
	const std::vector<Card>& getOwnedCards() const;

	bool getHasConflictPawn() const;

	int getRemainingCoins() const;

	int getMilitaryVictoryPoints() const;
	int getBuildingVictoryPoints() const;
	int getWonderVictoryPoints() const;
	int getProgressVictoryPoints() const;
};
}