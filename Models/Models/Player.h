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



};
}

