export module Models.Card; 

import <functional>;
import <array>; 
import <cstdint>;
import <unordered_map>;
import <string>;
import Models.ResourceType;
import Models.CoinWorthType;
import Models.ColorType;
import Models.ICard;
import <iostream>;

export namespace Models
{
	export class __declspec(dllexport) Card : ICard
	{
	private:
		std::string m_name;
		std::unordered_map<ResourceType, uint8_t> m_resourceCost{};
		uint8_t m_victoryPoints{};
		CoinWorthType m_coinWorth{};
		uint8_t m_coinReward{};
		std::string m_caption{};
		ColorType m_color{};
		std::vector<std::function<void()>> m_onPlayActions{};
		std::vector<std::function<void()>> m_onDiscardActions{};
		bool m_isVisible{ false };
		bool m_isAvailable{ false };
		//std::string m_modelPath = "UI path placeholder";

	public:
		Card() = default;
		Card(const Card& other) = delete;
		virtual Card& operator=(const Card& other) = delete;
		Card(Card&& other);
		virtual Card& operator=(Card&& other);
		virtual ~Card() = default;

		virtual void onPlay();
		virtual void onDiscard();

		const std::string& getName() const;
		const std::unordered_map<ResourceType, uint8_t>& getResourceCost() const;
		const uint8_t& getVictoryPoints() const;
		const CoinWorthType& getCoinWorth() const;
		const uint8_t& getCoinReward() const;
		const std::string& getCaption() const;
		const ColorType& getColor() const;
		const std::vector<std::function<void()>>& getOnPlayActions() const;
		const std::vector<std::function<void()>>& getOnDiscardActions() const;
		const bool& isVisible() const;
		const bool& isAvailable() const;

		void setName(const std::string& name);
		void setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		void setVictoryPoints(const uint8_t& victoryPoints);
		void setCoinWorth(const CoinWorthType& coinWorth);
		void setCoinReward(const uint8_t& coinReward);
		void setCaption(const std::string& caption);
		void setColor(const ColorType& color);
		void addOnPlayAction(const std::function<void()>& action);
		void addOnDiscardAction(const std::function<void()>& action);
		void setIsVisible(const bool& isVisible);
		void setIsAvailable(const bool& isAvailable);

		virtual void displayCardInfo() override;
	};

	export class __declspec(dllexport) CardBuilder
	{
		Card m_card;
	public:
		CardBuilder& setName(const std::string& name);
		CardBuilder& setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		CardBuilder& setVictoryPoints(const uint8_t& victoryPoints);
		CardBuilder& setCoinWorth(const CoinWorthType& coinWorth);
		CardBuilder& setCoinReward(const uint8_t& coinReward);
		CardBuilder& setCaption(const std::string& caption);
		CardBuilder& setColor(const ColorType& color);
		CardBuilder& addOnPlayAction(const std::function<void()>& action);
		CardBuilder& addOnDiscardAction(const std::function<void()>& action);
		Card build();
	};
}