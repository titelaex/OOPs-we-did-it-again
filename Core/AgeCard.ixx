export module Core.AgeCard;

import Models.Card;

export namespace Core {
	export class AgeCard {
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