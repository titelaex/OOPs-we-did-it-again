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
		uint8_t m_shieldPoints;
		ResourceType m_resourceProduction;
		bool m_playSecondTurn = false;
		bool m_drawProgressTokens = false;
		bool m_chooseAndConstructBuilding = false;
		bool m_discardCardFromOpponent = false; // brown or grey
		ColorType m_discardedCardColor;
	public:
	
		Wonder() = delete;
		Wonder(const Wonder& other) = delete;
		Wonder& operator=(const Wonder& other) = delete;
		Wonder(Wonder&& other) = default;
		Wonder& operator=(Wonder&& other) = default;
		~Wonder() = default;

		Wonder(
			const std::string& name,
			const std::unordered_map<ResourceType, uint8_t>& resourceCost,
			uint8_t victoryPoints,
			CoinWorthType coinWorth,
			uint8_t coinReward,
			const std::string& caption,
			ColorType color,
			bool isVisible,
			const std::string& modelPath,

			ResourceType resourceProduction,
			uint8_t shieldPoints,
			uint8_t playerReceivesMoney,
			uint8_t opponentLosesMoney,
			bool discardCardFromOpponent,
			bool playSecondTurn,
			bool drawProgressTokens,
			bool chooseAndConstructBuilding,
			ColorType discardedCardColor
		);

		uint8_t getPlayerReceivesMoney() const { return m_playerReceivesMoney; }
		uint8_t getOpponentLosesMoney() const { return m_opponentLosesMoney; }
		bool getPlaySecondTurn() const { return m_playSecondTurn; }
		bool getDrawProgressTokens() const { return m_drawProgressTokens; }
		bool getChooseAndConstructBuilding() const { return m_chooseAndConstructBuilding; }
		bool getDiscardCardFromOpponent() const { return m_discardCardFromOpponent; }

		void setPlayerReceivesMoney(uint8_t amount) { m_playerReceivesMoney = amount; }
		void setOpponentLosesMoney(uint8_t amount) { m_opponentLosesMoney = amount; }
		void setPlaySecondTurn(bool value) { m_playSecondTurn = value; }
		void setDrawProgressTokens(bool value) { m_drawProgressTokens = value; }
		void setChooseAndConstructBuilding(bool value) { m_chooseAndConstructBuilding = value; }
		void setDiscardCardFromOpponent(bool value) { m_discardCardFromOpponent = value; }

		void displayCardInfo() override;


		
	};
	export std::ostream& operator<<(std::ostream& os, const Wonder& card);
}