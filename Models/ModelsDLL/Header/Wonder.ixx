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

	public:
		static uint8_t wondersBuilt; //increase when playCardWonder()

	public:

		Wonder() = delete;
		Wonder(const Wonder& other) = delete;
		Wonder& operator=(const Wonder& other) = delete;
		Wonder(Wonder&& other) = default;
		Wonder& operator=(Wonder&& other) = default;
		~Wonder() = default;

		const uint8_t& getOpponentLosesMoney() const;
		const uint8_t& getShieldPoints() const;
		const ResourceType& getResourceProduction() const;
		const std::bitset<5>& getFlags() const;

		void setOpponentLosesMoney(uint8_t amt);
		void setShieldPoints(uint8_t pts);
		void setResourceProduction(ResourceType r);
		void setFlags(const std::bitset<5>& flags);

		void displayCardInfo() override;
		void onDiscard() override;

	};
	export std::ostream& operator<<(std::ostream& os, const Wonder& card);

	export class __declspec(dllexport) WonderBuilder : public CardBuilder
	{
		Wonder m_card;
	public:
		WonderBuilder& setName(const std::string& name);
		WonderBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		WonderBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		WonderBuilder& setCoinWorth(const CoinWorthType& coinWorth);
		WonderBuilder& setCoinReward(const uint8_t& coinReward);
		WonderBuilder& setCaption(const std::string& caption);
		WonderBuilder& setColor(const ColorType& color);
		WonderBuilder& setOpponentLosesMoney(const uint8_t& amt);
		WonderBuilder& setShieldPoints(const uint8_t& pts);
		WonderBuilder& setResourceProduction(const ResourceType& r);
		WonderBuilder& setFlags(const std::bitset<5>& flags);
		WonderBuilder& setDiscardedCardColor(const ColorType& color);
		WonderBuilder& addOnPlayAction(const std::function<void()>& action);
		Wonder build();
	};
}