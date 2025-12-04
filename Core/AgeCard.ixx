export module Core.AgeCard;

import Models.Card;
import Core.Card;
export namespace Core {
	export class AgeCard :public Core::Card
	{
	public:
		Models::Card m_ageCard;
	private:
		bool isVisible = false;
		bool isAccessible = false;
	public:
		const bool& getCardVisible() const;
		void setCardVisibility(const bool& visible);
		const bool& getCardAccessible() const;
		void setCardAccessibility(const bool& accessible);
	};
}