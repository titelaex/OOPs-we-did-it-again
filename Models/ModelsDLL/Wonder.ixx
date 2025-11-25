export module Models.Wonder;

import Models.ResourceType;
import Models.Token;
import Models.Card;
import <string>;
import <vector>;
import <random>;
import <algorithm>;
import <iterator>;
import <bitset>;

namespace Models
{
	export class __declspec(dllexport) Wonder : public Card
	{
	private:
		uint8_t m_playerReceivesMoney = 0; // coins the player receives when constructing this wonder
		uint8_t m_opponentLosesMoney = 0;
		uint8_t m_shieldPoints = 0;
		ResourceType m_resourceProduction;
		// Flags for wonder special effects. Stored as a compact bitset on every tile so the tile
		// representation only requires a single small field for all boolean effect toggles.
		// Bit positions:
		// 0 = play second turn, 1 = draw progress tokens, 2 = choose & construct building,
		// 3 = discard card from opponent, 4 = is constructed
		std::bitset<5> m_flags;
		ColorType m_discardedCardColor;
	
		bool reserved_padding = false; // reserved for future use
	public:
	
		Wonder() = delete;
		Wonder(const Wonder& other) = default;
		Wonder& operator=(const Wonder& other) = default;
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
		uint8_t getShieldPoints() const { return m_shieldPoints; }
		ResourceType getResourceProduction() const { return m_resourceProduction; }

		bool getPlaySecondTurn() const { return m_flags.test(0); }
		bool getDrawProgressTokens() const { return m_flags.test(1); }
		bool getChooseAndConstructBuilding() const { return m_flags.test(2); }
		bool getDiscardCardFromOpponent() const { return m_flags.test(3); }
		bool getIsConstructed() const { return m_flags.test(4); }

		void setPlayerReceivesMoney(uint8_t amt) { m_playerReceivesMoney = amt; }
		void setOpponentLosesMoney(uint8_t amt) { m_opponentLosesMoney = amt; }
		void setShieldPoints(uint8_t pts) { m_shieldPoints = pts; }
		void setResourceProduction(ResourceType r) { m_resourceProduction = r; }

		void setPlaySecondTurn(bool v) { m_flags.set(0, v); }
		void setDrawProgressTokens(bool v) { m_flags.set(1, v); }
		void setChooseAndConstructBuilding(bool v) { m_flags.set(2, v); }
		void setDiscardCardFromOpponent(bool v) { m_flags.set(3, v); }
		void setIsConstructed(bool v) { m_flags.set(4, v); }

		void displayCardInfo() override;
		static uint8_t wondersBuilt;

		
	};
	export std::ostream& operator<<(std::ostream& os, const Wonder& card);
}