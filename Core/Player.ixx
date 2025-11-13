export module Player;

import Wonder;
import Card;
import Player;
import Token;

export namespace Core
{
	export class Player
	{
	public:
		virtual void playCardWonder(Models::Wonder& wonder, Models::Card& ageCard, Player& opponent,
			std::vector<Models::Token>& discardedTokens, std::vector<Models::Card>& discardedCards,
			uint8_t& totalWondersBuilt)
		{
			// 1. Check if Wonder is already built
			if (wonder.GetIsVisible())
			{
				std::cout << "Wonder \"" << wonder.GetName() << "\" is already constructed.\n";
				return;
			}

			// 2. Check affordability (you'll need to implement this logic)
			if (!canAffordWonder(wonder, opponent))
			{
				std::cout << "Cannot afford Wonder \"" << wonder.GetName() << "\".\n";
				return;
			}

			// 3. Pay cost (resources + coins)
			payForWonder(wonder, opponent);

			// 4. Use Age card to build Wonder
			ageCard.toggleVisibility(); // mark as discarded

			// 5. Trigger Wonder effects
			if (wonder.kplayerReceivesMoney > 0)
				wonder.m_receiveMoneyAction(*this);

			if (wonder.kopponentLosesMoney > 0)
				wonder.m_opponentLosesMoneyAction(opponent);

			if (wonder.kplaySecondTurn)
				wonder.m_playSecondTurnAction(*this);

			if (wonder.kdrawProgressTokens)
				wonder.m_drawProgressTokenAction(discardedTokens);

			if (wonder.kchooseAndConstructBuilding)
				wonder.m_chooseAndConstructBuildingAction(discardedCards);

			if (wonder.kdiscardBrownCardFromOpponent)
				wonder.m_discardBrownCardFromOpponentAction(opponent);

			// 6. Add Wonder to player's built list
			addWonder(wonder);
			wonder.SetIsVisible(true); // mark as constructed

			// 7. Check if 7 Wonders have been built
			++totalWondersBuilt;
			if (totalWondersBuilt == 7)
			{
				discardRemainingWonder(); // implement this in your game engine
			}

			std::cout << "Wonder \"" << wonder.GetName() << "\" constructed successfully.\n";
		}

	private:
		// Placeholder for affordability check
		bool canAffordWonder(const Models::Wonder& wonder, const Player& opponent)
		{
			// Implement resource + coin check logic here
			return true;
		}

		// Placeholder for payment logic
		void payForWonder(const Models::Wonder& wonder, const Player& opponent)
		{
			// Implement resource deduction or coin trading logic here
		}

		// Placeholder for discarding the 8th Wonder
		void discardRemainingWonder()
		{
			// Implement logic to remove the last unbuilt Wonder from the game
		}
	};
}