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
		virtual void playCardWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Card>& ageCard, std::unique_ptr<Models::Player>& opponent,
			std::vector<Models::Token>& discardedTokens, std::vector<std::unique_ptr<Models::Card>>& discardedCards,
			uint8_t& totalWondersBuilt);
		virtual void playCardBuilding(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player > &opponent);

	private:

		bool canAffordWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Player>& opponent);
		void payForWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Player>& opponent);
		void discardRemainingWonder();
		bool canAffordCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent);
		void payForCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent);
		void applyCardEffects(std::unique_ptr<Models::Card>& card);
	};
}