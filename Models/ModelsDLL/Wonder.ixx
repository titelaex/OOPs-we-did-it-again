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
		uint8_t m_opponentLosesMoney = 0;
		uint8_t m_shieldPoints = 0;
		ResourceType m_resourceProduction;
		// Flags for wonder special effects. Stored as a compact bitset on every tile so the tile
		// representation only requires a single small field for all boolean effect toggles.
		// Bit positions:
		// 0 = play second turn, 1 = draw progress tokens, 2 = choose & construct building,
		// 3 = discard browm card from opponent, 4 - is constructed
		std::bitset<5> m_flags;
		ColorType m_discardedCardColor;

		bool reserved_padding = false; // reserved for future use

	public:
		static uint8_t wondersBuilt; //increase when playCardWonder()

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

			uint8_t opponentLosesMoney,
			uint8_t shieldPoints,
			ResourceType resourceProduction,
			bool playSecondTurn,
			bool drawProgressTokens,
			bool discardCardFromOpponent,
			bool chooseAndConstructBuilding,
			ColorType discardedCardColor
		);

		uint8_t getOpponentLosesMoney() const;
		uint8_t getShieldPoints() const;
		ResourceType getResourceProduction() const;

		bool getPlaySecondTurn() const;
		bool getDrawProgressTokens() const;
		bool getChooseAndConstructBuilding() const;
		bool getDiscardCardFromOpponent() const;
		bool getIsConstructed() const;

		void setOpponentLosesMoney(uint8_t amt);
		void setShieldPoints(uint8_t pts);
		void setResourceProduction(ResourceType r);

		void setPlaySecondTurn(bool v);
		void setDrawProgressTokens(bool v);
		void setChooseAndConstructBuilding(bool v);
		void setDiscardCardFromOpponent(bool v);
		void setIsConstructed(bool v);

		void displayCardInfo() override;


	};
	export std::ostream& operator<<(std::ostream& os, const Wonder& card);
}