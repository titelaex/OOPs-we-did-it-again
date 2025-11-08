#pragma once
module Models.Player;

namespace Models
{
	class ICard
	{
	public:
		virtual ~ICard() = default;

		virtual void dealCard(Player player) = 0;
		//jucatorul ia cartea din vectorul de carti vizibile 
		virtual void toggleVisibility() = 0;
		virtual void playCardWonder() = 0;
		virtual void playCardBuilding() = 0;
		virtual void burnCard() = 0;
		virtual void displayCardInfo() = 0;
	};

}

