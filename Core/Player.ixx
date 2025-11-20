export module Core.Player;

import Models.Wonder;
import Models.Card;
import Models.Player;
import Models.Token;
import Models.LinkingSymbolType;
import <iostream>;
import <vector>;

export namespace Core
{
	export class Player
	{
	public:
		Models::Player m_player;
		virtual void playCardWonder(Models::Wonder& wonder, Models::Card& ageCard, Models::Player& opponent,
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
			if (wonder.getPlayerReceivesMoney > 0)
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
			m_player.addWonder(wonder);
			wonder.SetIsVisible(true); // mark as constructed

			// 7. Check if 7 Wonders have been built
			++totalWondersBuilt;
			if (totalWondersBuilt == 7)
			{
				discardRemainingWonder();
			}

			std::cout << "Wonder \"" << wonder.GetName() << "\" constructed successfully.\n";
		}
		virtual void playCardBuilding(Models::Card& card, Models::Player& opponent)
		{
			if (!card.GetIsVisible())
			{
				std::cout << "Card \"" << card.GetName() << "\" is not accessible.\n";
				return;
			}

			// Check for chain construction
			if (card.GetRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
			{
				for (const auto& ownedCard : m_player.getOwnedCards())
				{
					if (ownedCard.GetHasLinkingSymbol() == card.GetRequiresLinkingSymbol())
					{
						m_player.addCard(card);
						card.SetIsVisible(false);
						std::cout << "Card \"" << card.GetName() << "\" constructed for free via chain.\n";
						applyCardEffects(card);
						return;
					}
				}
			}

			// Check if card is free
			if (card.GetResourceCost().empty() && card.GetCoinValue() == 0)
			{
				m_player.addCard(card);
				card.SetIsVisible(false);
				std::cout << "Card \"" << card.GetName() << "\" constructed for free.\n";
				applyCardEffects(card);
				return;
			}

			// Check affordability
			if (!canAffordCard(card, opponent))
			{
				std::cout << "Cannot afford to construct \"" << card.GetName() << "\".\n";
				return;
			}

			// Pay cost
			payForCard(card, opponent);

			// Construct card
			m_player.addCard(card);
			card.SetIsVisible(false);
			std::cout << "Card \"" << card.GetName() << "\" constructed.\n";
			applyCardEffects(card);
		}

	private:

		bool canAffordWonder(const Models::Wonder& wonder, const Models::Player& opponent)
		{
			const auto& cost = wonder.GetResourceCost();
			const auto& ownPermanent = m_player.getOwnedPermanentResources();
			const auto& ownTrading = m_player.getOwnedTradingResources();
			const auto& opponentPermanent = opponent.getOwnedPermanentResources();

			uint8_t totalAvailableCoins = m_player.totalCoins(m_player.getRemainingCoins());

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

		void payForWonder(const Models::Wonder& wonder, const Models::Player& opponent)
		{
			const auto& cost = wonder.GetResourceCost();
			const auto& ownPermanent = m_player.getOwnedPermanentResources();
			const auto& ownTrading = m_player.getOwnedTradingResources();
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

			m_player.subtractCoins(totalCoinsToPay);
		}

		void discardRemainingWonder(std::vector<Models::Wonder>& allWonders)
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
		bool canAffordCard(const Models::Card& card, const Models::Player& opponent)
		{
			// 1. Free card
			if (card.GetResourceCost().empty() && card.GetCoinValue() == 0)
				return true;

			// 2. Chain construction
			if (card.GetRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
			{
				for (const auto& ownedCard : m_player.getOwnedCards())
				{
					if (ownedCard.GetHasLinkingSymbol() == card.GetRequiresLinkingSymbol())
						return true;
				}
			}

			// 3. Resource cost
			const auto& cost = card.GetResourceCost();
			const auto& ownPermanent = m_player.getOwnedPermanentResources();
			const auto& ownTrading = m_player.getOwnedTradingResources();
			const auto& opponentProduction = opponent.getOwnedPermanentResources();

			uint8_t availableCoins = m_player.totalCoins(m_player.getRemainingCoins());

			for (const auto& [resource, requiredAmount] : cost)
			{
				uint8_t produced = 0;
				if (ownPermanent.contains(resource)) produced += ownPermanent.at(resource);
				if (ownTrading.contains(resource)) produced += ownTrading.at(resource);

				if (produced >= requiredAmount)
					continue;

				uint8_t missing = requiredAmount - produced;
				uint8_t opponentAmount = opponentProduction.contains(resource) ? opponentProduction.at(resource) : 0;
				uint8_t costPerUnit = 2 + opponentAmount;

				uint8_t totalCost = costPerUnit * missing;
				if (availableCoins < totalCost)
					return false;

				availableCoins -= totalCost;
			}

			// 4. Coin cost (if any)
			if (card.GetCoinValue() > availableCoins)
				return false;

			return true;
		}
		void payForCard(const Models::Card& card, const Models::Player& opponent)
		{
			const auto& cost = card.GetResourceCost();
			const auto& ownPermanent = m_player.getOwnedPermanentResources();
			const auto& ownTrading = m_player.getOwnedTradingResources();
			const auto& opponentProduction = opponent.getOwnedPermanentResources();

			uint8_t totalCoinsToPay = 0;

			// 1. Pay for missing resources via trading
			for (const auto& [resource, requiredAmount] : cost)
			{
				uint8_t produced = 0;
				if (ownPermanent.contains(resource)) produced += ownPermanent.at(resource);
				if (ownTrading.contains(resource)) produced += ownTrading.at(resource);

				if (produced >= requiredAmount)
					continue;

				uint8_t missing = requiredAmount - produced;
				uint8_t opponentAmount = opponentProduction.contains(resource) ? opponentProduction.at(resource) : 0;
				uint8_t costPerUnit = 2 + opponentAmount;

				totalCoinsToPay += costPerUnit * missing;
			}

			// 2. Add coin cost from the card itself
			totalCoinsToPay += card.GetCoinValue();

			// 3. Deduct coins from player
			m_player.subtractCoins(totalCoinsToPay);

			std::cout << "Paid " << static_cast<int>(totalCoinsToPay) << " coins to construct \"" << card.GetName() << "\".\n";
		}
		void applyCardEffects(Models::Card& card)
		{
			//de implementat
		}
	};
}