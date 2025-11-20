export module Core.Player;

import Models.Wonder;
import Models.Card;
import Models.Player; // concrete player from Models
import Models.Token;
import Models.LinkingSymbolType;
import Models.ResourceType; // added for ResourceType in unordered_map declarations
import <iostream>;
import <vector>;
import <unordered_map>;
import <tuple>;

export namespace Core
{
	export class Player
	{
	public:
		virtual void playCardWonder(Models::Wonder& wonder, Models::Card& ageCard, Player& opponent,
			std::vector<Models::Token>& discardedTokens, std::vector<Models::Card>& discardedCards,
			uint8_t& totalWondersBuilt)
		{
			if (wonder.GetIsVisible())
			{
				std::cout << "Wonder \"" << wonder.GetName() << "\" is already constructed.\n";
				return;
			}

			if (!canAffordWonder(wonder, opponent))
			{
				std::cout << "Cannot afford Wonder \"" << wonder.GetName() << "\".\n";
				return;
			}

			payForWonder(wonder, opponent);
			ageCard.toggleVisibility();

			if (wonder.PlayerReceivesMoney() > 0)
				wonder.m_receiveMoneyAction(reinterpret_cast<Models::Player&>(*this));
			if (wonder.OpponentLosesMoney() > 0)
				wonder.m_opponentLosesMoneyAction(reinterpret_cast<Models::Player&>(opponent));
			if (wonder.PlaySecondTurn())
				wonder.m_playSecondTurnAction(reinterpret_cast<Models::Player&>(*this));
			if (wonder.DrawProgressTokens())
				wonder.m_drawProgressTokenAction(discardedTokens);
			if (wonder.ChooseAndConstructBuilding())
				wonder.m_chooseAndConstructBuildingAction(discardedCards);
			if (wonder.DiscardBrownCardFromOpponent())
				wonder.m_discardBrownCardFromOpponentAction(reinterpret_cast<Models::Player&>(opponent));

			addWonder(wonder);
			wonder.SetIsVisible(true);
			++totalWondersBuilt;
			if (totalWondersBuilt == 7)
			{
				// placeholder for discardRemainingWonder
			}
			std::cout << "Wonder \"" << wonder.GetName() << "\" constructed successfully.\n";
		}

		virtual void playCardBuilding(Models::Card& card, Player& opponent)
		{
			if (!card.GetIsVisible())
			{
				std::cout << "Card \"" << card.GetName() << "\" is not accessible.\n";
				return;
			}

			if (card.GetRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
			{
				for (const auto& ownedCard : getOwnedCards())
				{
					if (ownedCard.GetHasLinkingSymbol() == card.GetRequiresLinkingSymbol())
					{
						addCard(card);
						card.SetIsVisible(false);
						std::cout << "Card \"" << card.GetName() << "\" constructed for free via chain.\n";
						applyCardEffects(card);
						return;
					}
				}
			}

			if (card.GetResourceCost().empty() && card.GetCoinValue() == 0)
			{
				addCard(card);
				card.SetIsVisible(false);
				std::cout << "Card \"" << card.GetName() << "\" constructed for free.\n";
				applyCardEffects(card);
				return;
			}

			if (!canAffordCard(card, opponent))
			{
				std::cout << "Cannot afford to construct \"" << card.GetName() << "\".\n";
				return;
			}

			payForCard(card, opponent);
			addCard(card);
			card.SetIsVisible(false);
			std::cout << "Card \"" << card.GetName() << "\" constructed.\n";
			applyCardEffects(card);
		}

	private:
		bool canAffordWonder(const Models::Wonder& wonder, const Player& opponent);
		void payForWonder(const Models::Wonder& wonder, const Player& opponent);
		bool canAffordCard(const Models::Card& card, const Player& opponent);
		void payForCard(const Models::Card& card, const Player& opponent);
		void applyCardEffects(Models::Card& card);

		virtual const std::vector<Models::Card>& getOwnedCards() const = 0;
		virtual const std::unordered_map<Models::ResourceType,uint8_t>& getOwnedPermanentResources() const = 0;
		virtual const std::unordered_map<Models::ResourceType,uint8_t>& getOwnedTradingResources() const = 0;
		virtual std::tuple<uint8_t,uint8_t,uint8_t> getRemainingCoins() const = 0;
		virtual uint8_t totalCoins(std::tuple<uint8_t,uint8_t,uint8_t>) const = 0;
		virtual void subtractCoins(uint8_t) = 0;
		virtual void addCard(const Models::Card&) = 0;
		virtual void addWonder(const Models::Wonder&) = 0;
	};
}