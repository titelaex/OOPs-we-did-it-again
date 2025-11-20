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
	private:
		uint8_t m_playerReceivesMoney;
		uint8_t m_opponentLosesMoney;
		bool m_playSecondTurn = false;
		bool m_drawProgressTokens = false;
		bool m_chooseAndConstructBuilding = false;
		bool m_discardBrownCardFromOpponent = false;
	public:
		uint8_t getPlayerReceivesMoney() const { return m_playerReceivesMoney; }
		uint8_t getOpponentLosesMoney() const { return m_opponentLosesMoney; }
		bool getPlaySecondTurn() const { return m_playSecondTurn; }
		bool getDrawProgressTokens() const { return m_drawProgressTokens; }
		bool getChooseAndConstructBuilding() const { return m_chooseAndConstructBuilding; }
		bool getDiscardBrownCardFromOpponent() const { return m_discardBrownCardFromOpponent; }

		/*void m_receiveMoneyAction(class Player& player);
		void m_opponentLosesMoneyAction(class Player& opponent);
		void m_playSecondTurnAction(class Player& player);
		std::vector<Token> randomTokenSelector(std::vector<Token>& discardedTokens);
		void m_drawProgressTokenAction(std::vector<Token>& discardedTokens);
		void m_chooseAndConstructBuildingAction(const std::vector<Card>& discardedCards);
		void m_discardBrownCardFromOpponentAction(class Player& opponent);*/
	};
}