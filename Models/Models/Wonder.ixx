export module Models.Wonder;

import Models.ResourceType;
import Models.Token;
import Models.Card;
import <string>;
import <vector>;
import <random>;
import <algorithm>;
import <iterator>;

namespace Models
{
	export class __declspec(dllexport) Wonder : public Card
	{
	protected:
		uint8_t kplayerReceivesMoney = 0;
		uint8_t kopponentLosesMoney = 0;
		bool kplaySecondTurn = false;
		bool kdrawProgressTokens = false;
		bool kchooseAndConstructBuilding = false;
		bool kdiscardBrownCardFromOpponent = false;
	public:
		uint8_t PlayerReceivesMoney() const { return kplayerReceivesMoney; }
		uint8_t OpponentLosesMoney() const { return kopponentLosesMoney; }
		bool PlaySecondTurn() const { return kplaySecondTurn; }
		bool DrawProgressTokens() const { return kdrawProgressTokens; }
		bool ChooseAndConstructBuilding() const { return kchooseAndConstructBuilding; }
		bool DiscardBrownCardFromOpponent() const { return kdiscardBrownCardFromOpponent; }

		void m_receiveMoneyAction(class Player& player);
		void m_opponentLosesMoneyAction(class Player& opponent);
		void m_playSecondTurnAction(class Player& player);
		std::vector<Token> randomTokenSelector(std::vector<Token>& discardedTokens);
		void m_drawProgressTokenAction(std::vector<Token>& discardedTokens);
		void m_chooseAndConstructBuildingAction(const std::vector<Card>& discardedCards);
		void m_discardBrownCardFromOpponentAction(class Player& opponent);
	};
}