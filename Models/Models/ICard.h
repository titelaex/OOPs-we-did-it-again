#pragma once

namespace Models
{
	class ICard
	{
	public:
		virtual ~ICard() = default;

		virtual void dealCard() = 0;
		virtual void toggleVisibility() = 0;
		virtual void playCard() = 0;
		virtual void burnCard() = 0;
		virtual void displayCardInfo() = 0;
	};

}

