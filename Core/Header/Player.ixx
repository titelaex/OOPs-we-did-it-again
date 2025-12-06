export module Core.Player;

import Models.Wonder;
import Models.Card;
import Models.Player;
import Models.Token;
import Models.ColorType;
import Models.LinkingSymbolType;
import <iostream>;
import <vector>;

export namespace Core
{
	export class Player
	{
	public:
		std::unique_ptr<Models::Player> m_player;
		virtual void playCardWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Card>& ageCard, std::unique_ptr<Models::Player>& opponent,
			std::vector<Models::Token>& discardedTokens, std::vector<std::unique_ptr<Models::Card>>& discardedCards);
		virtual void playCardBuilding(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player > &opponent);
		void takeCard(std::unique_ptr<Models::Card> card);
		void addCoins(uint8_t amt);
		void subtractCoins(uint8_t amt);
		void chooseWonder(std::vector<std::unique_ptr<Models::Wonder>>& availableWonders, uint8_t chosenIndex);
		void sellCard(std::unique_ptr<Models::Card>& ageCard, std::vector<std::unique_ptr<Models::Card>>& discardedCards);

	private:
		uint8_t countYellowCards();
		bool canAffordWonder(std::unique_ptr<Models::Wonder>& wonder, const std::unique_ptr<Models::Player>& opponent);
		void payForWonder(std::unique_ptr<Models::Wonder>& wonder,const std::unique_ptr<Models::Player>& opponent);
		void discardRemainingWonder(const std::unique_ptr<Models::Player>& opponent);
		bool canAffordCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent);
		void payForCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent);
		void applyCardEffects(std::unique_ptr<Models::Card>& card);
	};

	export void setCurrentPlayer(Player* p);
	export Player* getCurrentPlayer();
	export Player* getOpponentPlayer();
	export void playTurnForCurrentPlayer();
	export void drawTokenForCurrentPlayer();
	export void discardOpponentCardOfColor(Models::ColorType color);
	export void buildCardFromAge1Pool(size_t index);
}