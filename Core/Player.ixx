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
			uint8_t& totalWondersBuilt);
		virtual void playCardBuilding(Models::Card& card, Models::Player& opponent);

	private:

		bool canAffordWonder(const Models::Wonder& wonder, const Models::Player& opponent);
		void payForWonder(const Models::Wonder& wonder, const Models::Player& opponent);
		void discardRemainingWonder();
		bool canAffordCard(const Models::Card& card, const Models::Player& opponent);
		void payForCard(const Models::Card& card, const Models::Player& opponent);
		void applyCardEffects(Models::Card& card);
	};
}