export module Models.Card; 

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
		std::unordered_map<ResourceType, uint8_t> m_resourceCost;
		uint8_t m_victoryPoints{};
		CoinWorthType m_coinWorth = CoinWorthType::VALUE;
		uint8_t m_coinReward = 0;
		std::string m_caption;
		ColorType m_color{};
		bool m_isVisibile = false;
		bool m_isAccessible = false;
		std::string m_modelPath = "UI path placeholder";

	public:
		Card() = default;
		Card(const Card& other) = default;
		virtual Card& operator=(const Card& other) = default;
		Card(Card&& other);
		virtual Card& operator=(Card&& other);
		virtual ~Card() = default;

		Card(
			const std::string& name,
			const std::unordered_map<ResourceType, uint8_t>& resourceCost,
			uint8_t victoryPoints,
			CoinWorthType coinWorth,
			uint8_t coinReward,
			const std::string& caption,
			ColorType color,
			bool isVisible,
			const std::string& modelPath
		);

		const std::string& GetName() const;
		const std::unordered_map<ResourceType, uint8_t>& GetResourceCost() const;
		uint8_t GetVictoryPoints() const;
		CoinWorthType GetCoinWorth() const;
		uint8_t GetCoinReward() const;
		const std::string& GetCaption() const;
		ColorType GetColor() const;
		bool GetIsVisible() const;
		const std::string& GetModelPath() const;

		void SetName(const std::string& name);
		void SetResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost);
		void SetVictoryPoints(uint8_t victoryPoints);
		void SetCoinWorth(CoinWorthType coinWorth);
		void SetCoinReward(uint8_t coinReward);
		void SetCaption(const std::string& caption);
		void SetColor(ColorType color);
		void SetIsVisible(bool isVisible);
		void SetModelPath(const std::string& modelPath);
		
		virtual void toggleVisibility() override;
		virtual void toggleAccessibility() override;
		virtual void displayCardInfo() override;
	};
}