export module Wonder;

import ResourceType;
import Token;
import Card;
import Player;

import <string>;
import <vector>;
import <random>;
import <algorithm>;
import <iterator>;

namespace Models
{
	export class Wonder : Card
	{
	private:
		//const std::string m_wonderName; already in Card base class as m_name
		// const uint8_t m_VictoryPointsWorth;
		const uint8_t m_playerReceivesMoney;
		const uint8_t m_opponentLosesMoney;
		// const uint8_t ShieldPoints;
		//const std::unordered_map<ResourceType, uint8_t> m_resourceProduction; //produced each turn
		const bool m_playSecondTurn;
		const bool m_drawProgressTokens;
		const bool m_chooseAndConstructBuilding;
		const bool m_discardBrownCardFromOpponent;
	public:

		void m_receiveMoneyAction(Player& player)
		{
			///add m_playerReceivesMoney to the player's coin count
			player.addCoins(m_playerReceivesMoney);
		}

		void m_opponentLosesMoneyAction(Player& opponent)
		{
			///subtract m_opponentLosesMoney from the opponent's coin count
			opponent.subtractCoins(static_cast<uint8_t>(-m_opponentLosesMoney));
		}

		void m_playSecondTurnAction(Player& player)
		{
			///allow the player to play an additional turn immediately
			///this can be implemented by setting a flag in the game engine that allows the player to take another turn
		}


		std::vector<Token> randomTokenSelector(std::vector<Token>& discardedTokens)
		{
			const uint8_t tokensToSelect = 3;

			if (discardedTokens.size() <= tokensToSelect)
			{
				return discardedTokens;
			}

			std::vector<Token> selectedTokens;

			std::random_device seed;
			std::mt19937 generator(seed());

			std::sample(discardedTokens.begin(), discardedTokens.end(),
				std::back_inserter(selectedTokens),
				tokensToSelect,
				generator);

			return selectedTokens;

		}

		void m_drawProgressTokenAction(std::vector<Token>& discardedTokens)
		{
			///randomly Select 3 Tokens
			
			std::vector<Token> selectedTokens = randomTokenSelector(discardedTokens);

			///show them to the player - displayToken() (to be implemented in Token.ixx)
			
			/// player chooses one 
			
			Token chosenToken; /* get player choice from UI */;

			/// return the other two and eliminate the chosen one from the discardedTokens vector
			
			auto new_end = std::remove(discardedTokens.begin(), discardedTokens.end(), chosenToken);

			/// play the chosen token effect - playToken() (to be implemented in Token.ixx)

		}
		
		void m_chooseAndConstructBuildingAction(const std::vector<Card>& discardedCards)
		{
			///show the discarded cards to the player (CARDS DISCARDED DURING THE SETUP NOT INCLUDED)
			
			
			///player chooses one and builds it for free
			
			Card chosenCard; /* get player choice from UI */;

			/// eliminate the chosen card from the discardedCards vector and add it to the player's owned cards
		
			auto new_end = std::remove(discardedCards.begin(), discardedCards.end(), chosenCard);
		}

		void m_discardBrownCardFromOpponentAction(Player& opponent)
		{
			///show the brown cards owned by the opponent to the player
		
			///player chooses one to discard
			
			Card chosenCard; /* get player choice from UI */;

			///remove the chosen card from the opponent's owned cards and add it to the discarded pile
		
			auto new_end = std::remove(opponent.getOwnedCards().begin(), opponent.getOwnedCards().end(), chosenCard);

		}

	};
}