#pragma once

#include "Building.h";
#include "Wonder.h";
#include "Card.h";

#include <string>
#include <vector>

namespace Models
{
class Player
{
private:
	const std::string playerId; //login info / to track turns
	std::string playerUsername; //login info
	std::string password; //login info

	int positionOnBoard = 0;

	std::vector<Building> ownedBuildings; //Buildings needs to be its own class that inherits from Cards
	std::vector<Wonder> ownedWonders; //Wonders needs to be its own class that inherits from Cards
	std::vector<Card> ownedCards; ///maybe should be changed to AgeCards and create AgeCard class that inherits from Card???
	/// vector de ownedTokens ?
	bool hasConflictPawn = false;

	int remainingCoins = 7;

	// Scoreboard class ???? - rules
	int militaryVictoryPoints = 0;
	int buildindgVictoryPoints = 0;
	int wonderVictoryPoints = 0;
	int progressVictoryPoints = 0;

public:

	//setters
	void setPlayerId(const std::string& id);
	void setPlayerUsername(const std::string& username);
	void setPassword(const std::string& pwd);

	void setPositionOnBoard(int position);

	void addBuilding(const Building& building);
	void addWonder(const Wonder& wonder);
	void addCard(const Card& card);

	void setHasConflictPawn(bool hasPawn);
	void setRemainingCoins(int coins);

	void setMilitaryVictoryPoints(int points);
	void setBuildingVictoryPoints(int points);
	void setWonderVictoryPoints(int points);
	void setProgressVictoryPoints(int points);

	//getters
	const std::string& getPlayerId() const;
	const std::string& getPlayerUsername() const;
	const std::string& getPassword() const;

	int getPositionOnBoard() const;
	const std::vector<Building>& getOwnedBuildings() const;
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

