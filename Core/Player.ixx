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

			// 2. Check affordability 
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
				discardRemainingWonder();
			}

			std::cout << "Wonder \"" << wonder.GetName() << "\" constructed successfully.\n";
		}

	private:

		bool canAffordWonder(const Wonder& wonder, const Player& opponent)
		{
			const auto& cost = wonder.GetResourceCost();
			const auto& ownPermanent = getOwnedPermanentResources();
			const auto& ownTrading = getOwnedTradingResources();
			const auto& opponentPermanent = opponent.getOwnedPermanentResources();

			uint8_t totalAvailableCoins = totalCoins(getRemainingCoins());

			for (const auto& [resource, requiredAmount] : cost)
			{
				uint8_t produced = 0;
				if (ownPermanent.contains(resource)) produced += ownPermanent.at(resource);
				if (ownTrading.contains(resource)) produced += ownTrading.at(resource);

				if (produced >= requiredAmount)
					continue;

				uint8_t missing = requiredAmount - produced;
				uint8_t opponentProduction = opponentPermanent.contains(resource) ? opponentPermanent.at(resource) : 0;
				uint8_t costPerUnit = 2 + opponentProduction;
				uint8_t totalCost = costPerUnit * missing;

				if (totalAvailableCoins < totalCost)
					return false;

				totalAvailableCoins -= totalCost;
			}
			return true;
		}
		
		void payForWonder(const Wonder& wonder, const Player& opponent)
		{
			const auto& cost = wonder.GetResourceCost();
			const auto& ownPermanent = getOwnedPermanentResources();
			const auto& ownTrading = getOwnedTradingResources();
			const auto& opponentPermanent = opponent.getOwnedPermanentResources();

			uint8_t totalCoinsToPay = 0;

			for (const auto& [resource, requiredAmount] : cost)
			{
				uint8_t produced = 0;
				if (ownPermanent.contains(resource)) produced += ownPermanent.at(resource);
				if (ownTrading.contains(resource)) produced += ownTrading.at(resource);

				if (produced >= requiredAmount)
					continue;

				uint8_t missing = requiredAmount - produced;
				uint8_t opponentProduction = opponentPermanent.contains(resource) ? opponentPermanent.at(resource) : 0;
				uint8_t costPerUnit = 2 + opponentProduction;

				totalCoinsToPay += costPerUnit * missing;
			}

			subtractCoins(totalCoinsToPay);
		}

		void discardRemainingWonder(std::vector<Wonder>& allWonders)
		{
			uint8_t builtCount = 0;
			for (const auto& wonder : allWonders)
			{
				if (wonder.GetIsVisible())
					++builtCount;
			}

			if (builtCount >= 7)
			{
				for (auto& wonder : allWonders)
				{
					if (!wonder.GetIsVisible())
					{
						wonder.SetIsVisible(false); // Mark as discarded
						std::cout << "Wonder \"" << wonder.GetName() << "\" discarded as the 8th unbuilt Wonder.\n";
						break;
					}
				}
			}
		}
	};
}